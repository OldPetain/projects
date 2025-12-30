import os
import time
import torch
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import torch.nn.functional as F
from torch.utils.data import Dataset
import argparse
import random
from tensorboardX import SummaryWriter
import math
from PIL import Image


from data_loader import MerlDataset
from net_21paper_l3 import MLP
from test_and_rendering import loss

from test_and_rendering import utils
from test_and_rendering import data_loader_norepeat_train2pi
from test_and_rendering import render

def seed_torch(seed):
    random.seed(seed)
    os.environ['PYTHONHASHSEED'] = str(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    torch.cuda.manual_seed_all(seed)

def brdf_to_rgb(rvectors, brdf):
    hx = torch.reshape(rvectors[:, 0], (-1, 1))
    hy = torch.reshape(rvectors[:, 1], (-1, 1))
    hz = torch.reshape(rvectors[:, 2], (-1, 1))
    dx = torch.reshape(rvectors[:, 3], (-1, 1))
    dy = torch.reshape(rvectors[:, 4], (-1, 1))
    dz = torch.reshape(rvectors[:, 5], (-1, 1))

    theta_h = torch.atan2(torch.sqrt(hx ** 2 + hy ** 2), hz)
    theta_d = torch.atan2(torch.sqrt(dx ** 2 + dy ** 2), dz)
    phi_d = torch.atan2(dy, dx)
    wiz = torch.cos(theta_d) * torch.cos(theta_h) - \
          torch.sin(theta_d) * torch.cos(phi_d) * torch.sin(theta_h)
    rgb = brdf * torch.clamp(wiz, 0, 1)
    return rgb

def main(args):
    # 为numpy, python, 当前GPU设置随机数种子: 保证实验结果可重复
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    checkpoint_dir = args.output_dir + args.mat_name + '/' + args.checkpoint_fin_dir
    min_checkpoint_dir = args.output_dir + args.mat_name + '/' + args.min_checkpoint_fin_dir
    runs_dir = args.output_dir + args.mat_name + '/' + args.runs_fin_dir
    rendering_img_dir = args.output_dir + args.mat_name + '/' + args.rendering_img_fin_dir
    rendering_output_img_dir = args.output_dir + args.mat_name + '/' + args.rendering_output_img_fin_dir
    rendering_target_img_dir = args.output_dir + args.mat_name + '/' + args.rendering_target_img_fin_dir

    utils.createFolder(args.output_dir)
    utils.createFolder(args.output_dir + args.mat_name)
    utils.createFolder(checkpoint_dir)
    utils.createFolder(min_checkpoint_dir)
    utils.createFolder(runs_dir)
    utils.createFolder(rendering_img_dir)
    utils.createFolder(rendering_output_img_dir)
    utils.createFolder(rendering_target_img_dir)

    os.environ['CUDA_DEVICE_ORDER'] = 'PCI_BUS_ID'
    device = torch.device(args.gpu if torch.cuda.is_available() else "cpu")
    print(device)
    print('seed: ', args.seed)

    train_brdf_object = MerlDataset(args.merl_dir + args.mat_name + '.binary', args.train_batch_size, args.max_sample_num, device)

    test_data_object = data_loader_norepeat_train2pi.DataSet(args.merl_dir + args.mat_name, device, args.test_angles_dir,
                                                       args.testpd_pi_or_2pi)
    brdf_values = test_data_object.get_test_data_hd()
    test_angles_samples_t, test_brdf_samples_t = test_data_object.get_test_sample_pd2pi()


    test_li_pos = np.loadtxt(args.test_li_cam_pos_dir + 'test_li_pos.txt', delimiter=',')
    test_cam_pos = np.loadtxt(args.test_li_cam_pos_dir + 'test_cam_pos.txt', delimiter=',')

    test_li_pos_tensor = torch.from_numpy(test_li_pos).to(device).type(torch.float32)
    test_cam_pos_tensor = torch.from_numpy(test_cam_pos).to(device).type(torch.float32)
    brdf_values_tensor = torch.from_numpy(brdf_values).to(device).type(torch.float32)

    target_imgs = test_render_target_imgs(test_li_pos_tensor, test_cam_pos_tensor, args, brdf_values_tensor, rendering_target_img_dir, device)


    model = MLP(in_features=6, hid_features=21, out_features=3).to(device)

    optim = torch.optim.Adam(model.parameters(),
                             lr=args.lr,
                             betas=(0.9, 0.999),
                             eps=1e-15,  # eps=None raises error
                             weight_decay=0.0,
                             amsgrad=False)
    writer = SummaryWriter(logdir=runs_dir)

    begin_time = time.time()

    test_min_loss = 100
    test_min2_loss = 100
    test_min3_loss = 100

    test_img_min_loss = 100
    test_img_min2_loss = 100
    test_img_min3_loss = 100

    min_model_num = 0
    plot_test_BRDF_loss = []
    plot_test_img_loss = []

    for epoch in range(args.epochs):
        model.train()

        train_logMAE_cos_sum = 0  # batch-losses per epoch
        train_logMAE_sum = 0  # batch-losses per epoch
        train_brdf_object.shuffle_data()
        epochStart = time.time()
        num_batches = int(train_brdf_object.train_samples.shape[0] / args.train_batch_size)

        # iterate over batches
        num = 0
        for k in range(num_batches):
            optim.zero_grad()

            # get batch from MERL data, feed into model to get prediction
            mlp_input, groundTruth = train_brdf_object.get_trainbatch(k * train_brdf_object.bs)
            output = model(mlp_input).to(device)

            # convert to RGB data
            rgb_pred = brdf_to_rgb(mlp_input, output)
            rgb_true = brdf_to_rgb(mlp_input, groundTruth)

            train_loss_cos_object = loss.Loss(target=rgb_true, output=rgb_pred)
            logMAE_cos_loss = train_loss_cos_object.mean_absolute_logarithmic_error(y_true=rgb_true, y_pred=rgb_pred)

            train_loss_object = loss.Loss(target=groundTruth, output=output)
            logMAE_loss = train_loss_object.mean_absolute_logarithmic_error(y_true=groundTruth, y_pred=output)

            logMAE_cos_loss.backward()
            optim.step()

            train_logMAE_cos_sum = train_logMAE_cos_sum + logMAE_cos_loss
            train_logMAE_sum = train_logMAE_sum + logMAE_loss
            num = num + 1

        epoch_logMAE_cos_loss = train_logMAE_cos_sum / num
        epoch_logMAE_loss = train_logMAE_sum / num

        time_consume = time.time() - begin_time
        time_step = time_consume / (epoch + 1)
        time_total = time_step * args.epochs
        time_remained = time_total - time_consume

        h_time_remained = time_remained // 3600
        m_time_remained = time_remained % 3600 // 60
        s_time_remained = time_remained % 3600 % 60

        if (epoch + 1) % args.train_freq == 0:
            print('remained_time: [{}h {}m {}s] Epoch:{}  logMAE_cos_loss:{:.8f}   logMAE_loss:{:.8f}  lr:{:.5}   Time: {:.4f}s'.format(h_time_remained,
                                                                                               m_time_remained,
                                                                                               s_time_remained,
                                                                                               epoch + 1,
                                                                                               epoch_logMAE_cos_loss, epoch_logMAE_loss, args.lr, time.time() - epochStart))
            writer.add_scalar('epoch_logMAE_loss', epoch_logMAE_loss, epoch)
            writer.add_scalar('epoch_logMAE_cos_loss', epoch_logMAE_cos_loss, epoch)
            writer.add_scalar('seed', args.seed, epoch)
            writer.add_scalar('lr', args.lr, epoch)

        if (epoch + 1) % args.save_freq == 0 and (epoch + 1) > args.save_minepoch:
            torch.save(model.state_dict(), checkpoint_dir + 'model_epoch_%04d.pth' % (epoch + 1))

        if (epoch + 1) % args.test_freq == 0 and (epoch + 1 ) > args.test_minepoch:
            test_logMAE_loss = test(test_angles_samples_t, test_brdf_samples_t, model, writer, epoch)
            test_img_RMSE_loss, output_imgs = test_render_output_imgs(test_li_pos_tensor, test_cam_pos_tensor, model, writer, args, brdf_values_tensor, target_imgs, epoch, device)

            if (epoch + 1) > args.saveimg_minepoch:

                '''...保存前三个最小的loss的weight...'''
                if test_logMAE_loss < test_min_loss and min_model_num < 3:
                    min_loss_model_name = args.mat_name + '_min%d_loss_model' % (3 - min_model_num) + '_%06d_%.6f' % (
                        epoch + 1, test_logMAE_loss) + '.pth'
                    torch.save(model.state_dict(), min_checkpoint_dir + min_loss_model_name)
                    test_min_loss = test_logMAE_loss
                    min_model_num = min_model_num + 1
                    saveimgs(output_imgs, rendering_output_img_dir, epoch)

                elif test_logMAE_loss < test_min_loss and min_model_num >= 3:
                    models_name = os.listdir(min_checkpoint_dir)
                    models_name = sorted(models_name, reverse=True)
                    for model_name in models_name:
                        if model_name.startswith(args.mat_name + '_min3_'):
                            os.remove(min_checkpoint_dir + model_name)
                        if model_name.startswith(args.mat_name + '_min2_'):
                            n = len(args.mat_name)
                            os.rename(min_checkpoint_dir + model_name,
                                      min_checkpoint_dir + args.mat_name + '_min3_' + model_name[n + 6:])
                        if model_name.startswith(args.mat_name + '_min1_'):
                            n = len(args.mat_name)
                            os.rename(min_checkpoint_dir + model_name,
                                      min_checkpoint_dir + args.mat_name + '_min2_' + model_name[n + 6:])

                    min_loss_model_name = args.mat_name + '_min1_loss_model' + '_%06d_%.6f' % (
                    epoch + 1, test_logMAE_loss) + '.pth'
                    torch.save(model.state_dict(), min_checkpoint_dir + min_loss_model_name)
                    # 获得top3的test_logMAE_loss
                    test_min3_loss = test_min2_loss
                    test_min2_loss = test_min_loss
                    test_min_loss = test_logMAE_loss

                    min_model_num = min_model_num + 1
                    '''...保存前三个最小的loss的weight_end...'''

                    # 获得与test_min_loss对应的test_img_min_loss
                    test_img_min3_loss = test_img_min2_loss
                    test_img_min2_loss = test_img_min_loss
                    test_img_min_loss = test_img_RMSE_loss

                    saveimgs(output_imgs, rendering_output_img_dir, epoch)

                plot_test_img_loss.append(test_img_RMSE_loss)
            plot_test_BRDF_loss.append(test_logMAE_loss)

    plot_test_BRDF_loss = torch.tensor(plot_test_BRDF_loss)
    plot_test_img_loss = torch.tensor(plot_test_img_loss)
    plt.figure(1)
    plt.plot(plot_test_BRDF_loss)
    plt.savefig(args.output_dir + args.mat_name + "/" + args.mat_name + "_test_BRDF_logMAE_loss.png")
    plt.figure(2)
    plt.plot(plot_test_img_loss)
    plt.savefig(args.output_dir + args.mat_name + "/" + args.mat_name + "_test_img_RMSE_loss.png")

    test_min_loss = test_min_loss.cpu().numpy()
    test_min2_loss = test_min2_loss.cpu().numpy()
    test_min3_loss = test_min3_loss.cpu().numpy()
    test_img_min_loss = test_img_min_loss.cpu().numpy()
    test_img_min2_loss = test_img_min2_loss.cpu().numpy()
    test_img_min3_loss = test_img_min3_loss.cpu().numpy()
    min_loss_top3 = np.array(
        [test_min_loss, test_min2_loss, test_min3_loss, test_img_min_loss, test_img_min2_loss, test_img_min3_loss])
    np.savetxt(args.output_dir + args.mat_name + "/" + args.mat_name + '_test_min_img_min_top3_loss.txt', min_loss_top3,
               fmt='%.6f')

    consume_all_time = time.time() - begin_time

    h_time_consume = consume_all_time // 3600
    m_time_consume = consume_all_time % 3600 // 60
    s_time_consume = consume_all_time % 3600 % 60
    print('consume_all_time: {}h {}m {}s'.format(h_time_consume, m_time_consume, s_time_consume))

def test(test_angles_samples_t, test_brdf_samples_t, model, writer, epoch):
    model.eval()

    with torch.no_grad():
        output_brdf_sample_tp_n_t = model(test_angles_samples_t)
        target_brdf_sample_tp_n_t = test_brdf_samples_t

        output_brdf = output_brdf_sample_tp_n_t
        target_brdf = target_brdf_sample_tp_n_t
        loss_object = loss.Loss(target=target_brdf, output=output_brdf)
        logmae_loss = loss_object.test_log_loss()

        print('test_logmae_loss:{:.8f}'.format(logmae_loss))
        writer.add_scalar('test_logmae_loss', logmae_loss, epoch + 1)
        return logmae_loss

# 渲染平面图片，并输出normal map
def test_render_output_imgs(test_li_pos, test_cam_pos, model, writer, args, brdf_values, target_imgs, epoch, device):
    model.eval()
    output_imgs = torch.zeros((1, 256, 256, 3))
    with torch.no_grad():
        n = test_li_pos.shape[0] // args.n_test_rendering
        for j in range(n):
            test_li_pos_part = test_li_pos[j * args.n_test_rendering:(j + 1) * args.n_test_rendering, :]
            test_cam_pos_part = test_cam_pos[j * args.n_test_rendering:(j + 1) * args.n_test_rendering, :]
            render_object = render.RenderImages(brdf_values, args.size, args.lamp, args.res, test_li_pos_part,
                                                test_cam_pos_part, device)
            thetah, thetad, phid_fupipi, dist, wi_norm = render_object.test_compute_half_diff()  # (0,π/2), (0,π/2), (-π, π) 256*256*1
            phid = utils.normalize_phid_torch(phid_fupipi)  # pd(0，2π)
            hd = utils.rangles_to_rvectors_torch(thetah, thetad, phid)

            '''...model_21paper...'''
            hd = torch.reshape(hd, (-1, 6))
            output_brdf = model(hd)
            # reshape
            output_brdf = torch.reshape(output_brdf, (args.n_test_rendering, args.res, args.res, 3))
            output_result_gam = render_object.test_neuralBRDF_render_images(output_brdf, dist, wi_norm)
            output_result_gam = torch.clip(output_result_gam, 0.0, 1.0)
            output_result = output_result_gam ** (1 / 2.2)

            if j == 0:
                output_imgs = output_result
            else:
                output_imgs = torch.cat((output_imgs, output_result), dim=0)

        test_loss_object = loss.Loss(target_imgs, output_imgs)
        test_img_RMSE_avg_loss = test_loss_object.test_RMSE_loss()

        print('test_img_RMSE_loss:{:.8f}'.format(test_img_RMSE_avg_loss))
        writer.add_scalar('test_img_RMSE_loss', test_img_RMSE_avg_loss, epoch + 1)
        return test_img_RMSE_avg_loss, output_imgs

def test_render_target_imgs(test_li_pos, test_cam_pos, args, brdf_values, rendering_target_img_dir, device):
    n = test_li_pos.shape[0] // args.n_test_rendering
    target_imgs = torch.zeros((1, 256, 256, 3)).to(device)
    for i in range(n):
        test_li_pos_part = test_li_pos[i * args.n_test_rendering:(i + 1) * args.n_test_rendering, :]
        test_cam_pos_part = test_cam_pos[i * args.n_test_rendering:(i + 1) * args.n_test_rendering, :]
        render_object = render.RenderImages(brdf_values, args.size, args.lamp, args.res, test_li_pos_part,
                                            test_cam_pos_part, device)
        thetah, thetad, phid_fupipi, dist, wi_norm = render_object.test_compute_half_diff()  # (0,π/2), (0,π/2), (-π, π) 256*256*1
        # phid:[-pi, pi] ==> [0, pi]
        phid = (phid_fupipi < 0.0) * (phid_fupipi + math.pi) + (phid_fupipi > 0.0) * phid_fupipi

        '''...target_brdf'''
        target_result_gam = render_object.test_render_images(thetah, thetad, phid, dist, wi_norm)
        target_result_gam = torch.clip(target_result_gam, 0.0, 1.0)
        target_result = target_result_gam ** (1 / 2.2)

        target = target_result.clone().detach().cpu().numpy()

        img = target[0]
        img = np.clip(img * 255, 0, 255).astype(np.uint8)
        img_save = Image.fromarray(img)
        img_save.save(rendering_target_img_dir + 'target_%03d.png' % (i))

        if i == 0:
            target_imgs = target_result
        else:
            target_imgs = torch.cat((target_imgs, target_result), dim=0)
    return target_imgs

def saveimgs(output_result, rendering_output_img_dir, epoch):

    rendering_output_img_epoch_dir = rendering_output_img_dir + '%06d/'%(epoch + 1)
    utils.createFolder(rendering_output_img_epoch_dir)

    output = output_result.clone().detach().cpu().numpy()
    n = output.shape[0]
    for i in range(n):
        img = output[i]
        img = np.clip(img * 255, 0, 255).astype(np.uint8)
        img_save = Image.fromarray(img)
        img_save.save(rendering_output_img_epoch_dir + '%06d_sine_output_%03d.png' % (epoch + 1, i))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='pytorch --lzq')

    '''...output_h21_l4_hd_tp2_logMAE_lr1e4...'''
    parser.add_argument('--merl_dir', default='../../data/merl/')
    parser.add_argument('--mat_name', default='blue-acrylic')
    parser.add_argument('--test_li_cam_pos_dir', default='../../data/test_li_cam/')
    parser.add_argument('--test_angles_dir', default='../../data/test_th_td_pd2pi/')
    parser.add_argument('--gpu', default='cuda:0')

    parser.add_argument('--epochs', type=int, default=10)
    parser.add_argument('--max_sample_num', type=int, default=800000)
    parser.add_argument('--train_batch_size', type=int, default=512)
    parser.add_argument('--saveimg_minepoch', type=int, default=0) # save img 最小的epoch
    parser.add_argument('--train_freq', type=int, default=1)
    parser.add_argument('--save_freq', type=int, default=1)
    parser.add_argument('--save_minepoch', type=int, default=0) # save model 最小的epoch
    parser.add_argument('--test_freq', type=int, default=1)
    parser.add_argument('--test_minepoch', type=int, default=0) # test 最小的epoch
    parser.add_argument('--lr', type=float, default=5e-4)
    parser.add_argument('--testpd_pi_or_2pi', default='2pi')
    parser.add_argument('--output_dir', default='../../output/l3/21paper_h21l3_train2pi_b512_lr1e3_21paperrand/')
    parser.add_argument('--checkpoint_fin_dir', default='checkpoint/')
    parser.add_argument('--min_checkpoint_fin_dir', default='min/')
    parser.add_argument('--runs_fin_dir', default='runs/')
    parser.add_argument('--rendering_img_fin_dir', default='rendering/')
    parser.add_argument('--rendering_output_img_fin_dir', default='rendering/output/')
    parser.add_argument('--rendering_target_img_fin_dir', default='rendering/target/')
    parser.add_argument('--seed', type=int, default=0)

    parser.add_argument('--size', type=int, default=17.0)
    parser.add_argument('--lamp', default=1500)
    parser.add_argument('--res', default=256)
    parser.add_argument('--n_test_rendering', default=1)

    args = parser.parse_args()

    main(args)