'''
使用:
    seed_torch(args.seed)
    torch.backends.cudnn.enabled = True
    torch.backends.cudnn.benchmark = True

change:
merl_dir = '../data/merl_1_25/'
gpu = 'cuda:0'
lr = 5e-4
epochs = 1000
saveimg_minepoch = 100
save_minepoch = 100
test_minepoch = 0
train_freq = 1
test_freq = 1
save_freq = 10
epochs = 2000
cmd_main = './l3_21paperrand/main_21paper_h21l3_logMAE_train2pi_batch512_21paperrand.py'
output_dir = '../output/l3/21paper_h21l3_b512_lr1e3_21rand_1/'
'''

import os

merl_dir = '../data/merl_1_25/'
test_li_cam_pos_dir = '../data/test_li_cam/'
test_angles_dir = '../data/test_th_td_pd2pi/'

mats_name = os.listdir(merl_dir)
mats_name = sorted(mats_name)

gpu = 'cuda:0'

lr = 5e-4
testpd_pi_or_2pi = '2pi'

epochs = 1000
saveimg_minepoch = 100
save_minepoch = 100
test_minepoch = 0
train_freq = 1
test_freq = 1
save_freq = 10

'''...testing...'''
# epochs = 10
# saveimg_minepoch = 0
# save_minepoch = 0
# test_minepoch = 0
# train_freq = 1
# test_freq = 1
# save_freq = 1
'''...testing_end...'''


cmd_main = './l3_21paperrand/main_21paper_h21l3_logMAE_train2pi_batch512_21paperrand.py'
output_dir = '../output/l3/21paper_h21l3_b512_lr1e3_21rand_1/'

for mat_name in mats_name:
    name = mat_name[:-7]
    cmd = 'python '+ cmd_main \
        + ' --merl_dir ' + merl_dir \
        + ' --test_li_cam_pos_dir ' + test_li_cam_pos_dir \
        + ' --test_angles_dir ' + test_angles_dir \
        + ' --mat_name '+name\
        + ' --gpu ' + gpu\
        + ' --epochs ' + str(epochs)\
        + ' --saveimg_minepoch ' + str(saveimg_minepoch)\
        + ' --train_freq ' + str(train_freq) \
        + ' --save_freq ' + str(save_freq) \
        + ' --save_minepoch ' + str(save_minepoch) \
        + ' --test_freq ' + str(test_freq)\
        + ' --test_minepoch ' + str(test_minepoch)\
        + ' --lr ' + str(lr)\
        + ' --testpd_pi_or_2pi ' + testpd_pi_or_2pi\
        + ' --output_dir ' + output_dir
    print(cmd)
    os.system(cmd)