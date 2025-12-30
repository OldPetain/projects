'''
# 选择出数值最低的imgRMSE及对应的logMAE和model,保存选出的model
'''
import os
import numpy as np
import shutil

def create_folder(dir):
    if not os.path.exists(dir):
        os.mkdir(dir)

# dir = '../output/l3/21paper_h21l3_b512_top3/'
# out_dir = '../output/l3/21paper_h21l3_rendering/'

dir = '../output/l5_1/21paper_h21l5_b512_lr5e4_myrand_redfabric2_top3/'
out_dir = '../output/l5_1/21paper_h21l5_rendering/'


merls_name = os.listdir(dir)
min_name = ['min1', 'min2', 'min3']

create_folder(out_dir)

for merl_name in merls_name:

    merl_dir = dir + merl_name + '/'

    logMAE_imgRMSE_loss_txt = merl_dir + merl_name + '_test_min_img_min_top3_loss.txt'
    create_folder(out_dir+merl_name)
    shutil.copytree(dir+merl_name+'/rendering/target/', out_dir+merl_name+'/target/')

    if os.path.exists(logMAE_imgRMSE_loss_txt):
        logMAE_i = 0
        logMAE_imgRMSE = np.loadtxt(logMAE_imgRMSE_loss_txt)
        imgRMSE_min = logMAE_imgRMSE[3]
        for i in range(4,6):
            if logMAE_imgRMSE[i] < imgRMSE_min:
                imgRMSE_min = logMAE_imgRMSE[i]
                logMAE_i = i - 3
        print(merl_name)
        print(logMAE_imgRMSE)
        print(logMAE_i + 1)

        logMAE_min = logMAE_imgRMSE[logMAE_i]
        print('logMAE_min: ', logMAE_min)
        print('imgRMSE_min: ', imgRMSE_min)

        merl_models_name = os.listdir(merl_dir + 'min/')
        for merl_model_name in merl_models_name:
            if merl_model_name.split('_')[1] == min_name[logMAE_i]:
                print(merl_model_name)
                epoch = merl_model_name.split('_')[4]
                shutil.copytree(dir+merl_name+'/rendering/output/'+epoch, out_dir+merl_name+'/output/')
    else:
        print(merl_name + ': no ' + logMAE_imgRMSE_loss_txt )