'''
将21paper公布的model加载到网络中，测试采样点和平面图的结果
'''
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



def main(args):
    # 为numpy, python, 当前GPU设置随机数种子: 保证实验结果可重复
    # seed_torch(args.seed)
    np.random.seed(0)
    torch.manual_seed(0)

    rendering_img_dir = args.output_dir + args.mat_name + '/' + args.rendering_img_fin_dir
    rendering_output_img_dir = args.output_dir + args.mat_name + '/' + args.rendering_output_img_fin_dir
    rendering_target_img_dir = args.output_dir + args.mat_name + '/' + args.rendering_target_img_fin_dir

    utils.createFolder(args.output_dir)
    utils.createFolder(args.output_dir + args.mat_name)
    utils.createFolder(rendering_img_dir)
    utils.createFolder(rendering_output_img_dir)
    utils.createFolder(rendering_target_img_dir)

    os.environ['CUDA_DEVICE_ORDER'] = 'PCI_BUS_ID'
    device = torch.device(args.gpu if torch.cuda.is_available() else "cpu")

    test_data_object = data_loader_norepeat_train2pi.DataSet(args.merl_dir + args.mat_name, device,
                                                             args.test_angles_dir,
                                                             args.testpd_pi_or_2pi)
    brdf_values = test_data_object.get_test_data_hd()
    test_angles_samples_t, test_brdf_samples_t = test_data_object.get_test_sample_pd2pi()

    test_li_pos = np.loadtxt(args.test_li_cam_pos_dir + 'test_li_pos.txt', delimiter=',')
    test_cam_pos = np.loadtxt(args.test_li_cam_pos_dir + 'test_cam_pos.txt', delimiter=',')

    test_li_pos_tensor = torch.from_numpy(test_li_pos).to(device).type(torch.float32)
    test_cam_pos_tensor = torch.from_numpy(test_cam_pos).to(device).type(torch.float32)
    brdf_values_tensor = torch.from_numpy(brdf_values).to(device).type(torch.float32)

    model = MLP().to(device)

    # 加载npy模型
    if os.path.exists(args.model_dir+args.mat_name):
        model_dict_new = model.state_dict().copy()
        for i in range(1,4):
            trained_bias = np.load(args.model_dir+args.mat_name+'/'+args.mat_name+'_b%d.npy'%i)
            trained_bias_t = torch.from_numpy(trained_bias)
            # print('trained_bias.shape: ', trained_bias.shape)
            # print('model_dict_new(fc%d.bias).shape: '%i, model_dict_new['fc%d.bias'%i].shape)
            model_dict_new['fc%d.bias'%i] = trained_bias_t

        for i in range(1,4):
            trained_fc = np.load(args.model_dir+args.mat_name+'/'+args.mat_name+'_fc%d.npy'%i)
            trained_fc_t = torch.from_numpy(trained_fc)
            # print('trained_fc.shape: ', trained_fc.shape)
            # print('model_dict_new(fc%d.weight).shape: '%i, model_dict_new['fc%d.weight'%i].shape)
            model_dict_new['fc%d.weight' % i] = trained_fc_t.T

        model.load_state_dict(model_dict_new)

    model.eval()
    with torch.no_grad():
        test_logMAE_loss = test(model, test_angles_samples_t, test_brdf_samples_t)
        test_img_RMSE_loss = test_render(test_li_pos_tensor, test_cam_pos_tensor, model, args,
                                         rendering_output_img_dir, rendering_target_img_dir, brdf_values_tensor,
                                         device)

def test(model, test_angles_samples_t, test_brdf_samples_t):
    output_brdf_sample_tp_n_t = model(test_angles_samples_t)
    target_brdf_sample_tp_n_t = test_brdf_samples_t

    output_brdf = output_brdf_sample_tp_n_t
    target_brdf = target_brdf_sample_tp_n_t
    loss_object = loss.Loss(target=target_brdf, output=output_brdf)
    logmae_loss = loss_object.test_log_loss()

    print('test_logmae_loss:{:.8f}'.format(logmae_loss))

def test_render(test_li_pos, test_cam_pos, model, args, rendering_output_img_dir, rendering_target_img_dir,
                brdf_values, device):
    rendering_output_img_epoch_dir = rendering_output_img_dir + 'trained_out/'
    rendering_target_img_epoch_dir = rendering_target_img_dir + 'trained_out/'
    utils.createFolder(rendering_output_img_epoch_dir)
    utils.createFolder(rendering_target_img_epoch_dir)
    model.eval()
    with torch.no_grad():
        n = test_li_pos.shape[0] // args.n_test_rendering
        test_img_loss_sum = 0
        for j in range(n):
            test_li_pos_part = test_li_pos[j * args.n_test_rendering:(j + 1) * args.n_test_rendering, :]
            test_cam_pos_part = test_cam_pos[j * args.n_test_rendering:(j + 1) * args.n_test_rendering, :]
            render_object = render.RenderImages(brdf_values, args.size, args.lamp, args.res, test_li_pos_part,
                                                test_cam_pos_part, device)
            thetah, thetad, phid_pi, phid_fupipi, dist, wi_norm = render_object.test_compute_half_diff()  # (0,π/2), (0,π/2), (0, π) 256*256*1
            '''...(-pi,pi)==>(0,2pi)...'''
            phid_fupipi[phid_fupipi < 0] += 2 * math.pi
            phid_2pi = phid_fupipi
            '''...(-pi,pi)==>(0,2pi)_end...'''
            hd = utils.rangles_to_rvectors_torch(thetah, thetad, phid_2pi)

            '''...model_21paper...'''
            hd = torch.reshape(hd, (-1, 6))
            output_brdf = model(hd)
            # to01, untonemap, reshape
            output_brdf = torch.reshape(output_brdf, (args.n_test_rendering, args.res, args.res, 3))
            output_result = render_object.test_neuralBRDF_render_images(output_brdf, dist, wi_norm)
            output_result = torch.clip(output_result, 0.0, 1.0)

            output = output_result ** (1 / 2.2)
            output = output.clone().detach().cpu().numpy()
            for i in range(args.n_test_rendering):
                img = output[i]
                img = np.clip(img * 255, 0, 255).astype(np.uint8)
                img_save = Image.fromarray(img)
                img_save.save(rendering_output_img_epoch_dir + '21paper_output_%d_%d.png' % (j, i))

            '''...target_brdf'''
            target_result = render_object.test_render_images(thetah, thetad, phid_pi, dist, wi_norm)
            target_result = torch.clip(target_result, 0.0, 1.0)

            target = target_result ** (1 / 2.2)
            target = target.clone().detach().cpu().numpy()
            for i in range(args.n_test_rendering):
                img = target[i]
                img = np.clip(img * 255, 0, 255).astype(np.uint8)
                img_save = Image.fromarray(img)
                img_save.save(rendering_target_img_epoch_dir + '21paper_%d_%d.png' % (j, i))

            test_loss_object = loss.Loss(target_result, output_result)
            test_RMSE_loss = test_loss_object.test_RMSE_loss()
            test_img_loss_sum = test_img_loss_sum + test_RMSE_loss

        test_img_RMSE_avg_loss = test_img_loss_sum / n
        print('test_img_RMSE_loss:{:.8f}'.format(test_img_RMSE_avg_loss))
        return test_img_RMSE_avg_loss





if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='pytorch --lzq')

    '''...output_h21_l4_hd_tp2_logMAE_lr1e4...'''
    parser.add_argument('--merl_dir', default='./data/merl/')
    parser.add_argument('--mat_name', default='blue-acrylic')
    parser.add_argument('--test_li_cam_pos_dir', default='./data/test_li_cam/')
    parser.add_argument('--test_angles_dir', default='./data/test_th_td_pd2pi/')
    parser.add_argument('--gpu', default='cuda:1')

    parser.add_argument('--train_freq', type=int, default=1)
    parser.add_argument('--testpd_pi_or_2pi', default='2pi')
    parser.add_argument('--output_dir', default='./output_21paper_provided_model/')
    parser.add_argument('--model_dir', default='./21paper_provided_model/')
    parser.add_argument('--runs_fin_dir', default='runs/')
    parser.add_argument('--rendering_img_fin_dir', default='rendering/')
    parser.add_argument('--rendering_output_img_fin_dir', default='rendering/output/')
    parser.add_argument('--rendering_target_img_fin_dir', default='rendering/target/')
    parser.add_argument('--seed', default=1993324908)

    parser.add_argument('--size', type=int, default=17.0)
    parser.add_argument('--lamp', default=1500)
    parser.add_argument('--res', default=256)
    parser.add_argument('--n_test_rendering', default=10)

    args = parser.parse_args()

    if args.seed:
        pass
    else:
        args.seed = random.randint(0, 2 ** 31 - 1)
        print(args.seed)

    main(args)