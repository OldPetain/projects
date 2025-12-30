'''
# 选择出数值最低的imgRMSE及对应的logMAE和model,保存选出的model
'''
import os
import numpy as np
import shutil

def create_folder(dir):
    if not os.path.exists(dir):
        os.mkdir(dir)

'''h21l3'''
# dir = '../output/l3/21paper_h21l3_b512_lr1e3_21rand_top3/'
# txt_folder_dir = '../output/l3/21paper_h21l3_b512_lr5e4_21rand_txt/'
# out_dir = '../output/l3/21paper_h21l3_b512_lr5e4_21rand_min/'

'''h21l4'''
# dir = '../output/l4/21paper_h21l4_b512_lr5e4_21rand_top3/'
# txt_folder_dir = '../output/l4/21paper_h21l4_b512_lr5e4_21rand_txt/'
# out_dir = '../output/l4/21paper_h21l4_b512_lr5e4_21rand_min/'


'''h21l5'''
# dir = '../output/l5/21paper_h21l5_b512_lr5e4_21rand_top3/'
# txt_folder_dir = '../output/l5/21paper_h21l5_b512_lr5e4_21rand_txt/'
# out_dir = '../output/l5/21paper_h21l5_b512_lr5e4_21rand_min/'

'''h21l5'''
dir = '../output/l5_1/21paper_h21l5_b512_lr5e4_myrand_redfabric2_top3/'
txt_folder_dir = '../output/l5_1/21paper_h21l5_b512_lr5e4_21rand_txt/'
out_dir = '../output/l5_1/21paper_h21l5_b512_lr5e4_min/'


merls_name = os.listdir(dir)
min_name = ['min1', 'min2', 'min3']

create_folder(txt_folder_dir)
create_folder(out_dir)

f = open(txt_folder_dir+'min_logMAE_imgRMSE_loss.txt', 'w')
f.write("merl_name:   logMAE_min   imgRMSE_min")

logMAE_min_sum = 0
imgRMSE_min_sum = 0

for merl_name in merls_name:
    # # # 删除model_best
    # merl_dir = dir + merl_name + '/'
    # if os.path.exists(merl_dir+'model_best'):
    #     shutil.rmtree(merl_dir+'model_best')


    # 选择出数值最低的imgRMSE及对应的logMAE和model
    if merl_name == 'h21l5_min_logMAE_imgRMSE_loss.txt' or merl_name == 'readme.txt':
        continue

    merl_dir = dir + merl_name + '/'
    create_folder(merl_dir + 'model_min/')


    logMAE_imgRMSE_loss_txt = merl_dir + merl_name + '_test_min_img_min_top3_loss.txt'

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
        print(logMAE_i+1)

        logMAE_min = logMAE_imgRMSE[logMAE_i]
        print('logMAE_min: ', logMAE_min)
        print('imgRMSE_min: ', imgRMSE_min)

        f.write(f"{merl_name}:{logMAE_min},{imgRMSE_min}\n")


        merl_models_name = os.listdir(merl_dir + 'min/')
        for merl_model_name in merl_models_name:
            if merl_model_name.split('_')[1] == min_name[logMAE_i]:
                # shutil.copy(merl_dir + 'min/' + merl_model_name, merl_dir+'model_min/' + merl_model_name)
                shutil.copy(merl_dir + 'min/' + merl_model_name, out_dir + merl_model_name)

        logMAE_min_sum = logMAE_min + logMAE_min_sum
        imgRMSE_min_sum = imgRMSE_min + imgRMSE_min_sum
    else:
        print(merl_name + ': no ' + logMAE_imgRMSE_loss_txt )

logMAE_min_avg = logMAE_min_sum / 100
imgRMSE_min_avg = imgRMSE_min_sum / 100

print('logMAE_min_avg: ', logMAE_min_avg)
print('imgRMSE_min_avg: ', imgRMSE_min_avg)
f.write("logMAE_min_avg: " + f"{logMAE_min_avg}\n")
f.write("imgRMSE_min_avg: " + f"{imgRMSE_min_avg}\n")

f.close()
