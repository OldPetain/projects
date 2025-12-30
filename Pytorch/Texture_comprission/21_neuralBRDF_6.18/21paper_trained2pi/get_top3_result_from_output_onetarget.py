import os
import shutil

def create_folder(dir):
    if not os.path.exists(dir):
        os.mkdir(dir)

output_dir = '../output/l4/21paper_h21l4_b512_lr5e4_21rand/'
output_best_dir = '../output/l4/21paper_h21l4_b512_lr5e4_21rand_top3/'
rendering_dir = 'rendering/'
rendering_output_dir = 'rendering/output/'
rendering_target_dir = 'rendering/target/'

create_folder(output_best_dir)

mats_name = os.listdir(output_dir)
mats_name = sorted(mats_name)

for mat_name in mats_name:
    output_best_rendering_dir = output_best_dir + mat_name + '/' + rendering_dir
    output_best_rendering_output_dir = output_best_dir + mat_name + '/' + rendering_output_dir
    output_best_rendering_target_dir = output_best_dir + mat_name + '/' + rendering_target_dir

    create_folder(output_best_dir + mat_name)
    create_folder(output_best_rendering_dir)
    create_folder(output_best_rendering_output_dir)

    output_rendering_output_dir = output_dir + mat_name + '/' + rendering_output_dir
    output_rendering_target_dir = output_dir + mat_name + '/' + rendering_target_dir

    mat_dir = output_dir+mat_name+'/'
    best_mat_dir = output_best_dir+mat_name+'/'
    if not os.path.exists(best_mat_dir+'min'):
        shutil.copytree(mat_dir+'min', best_mat_dir+'min')
    if not os.path.exists(best_mat_dir+'runs'):
        shutil.copytree(mat_dir+'runs', best_mat_dir+'runs')
    if not os.path.exists(best_mat_dir+mat_name+'_test_BRDF_logMAE_loss.png'):
        shutil.copy(mat_dir+mat_name+'_test_BRDF_logMAE_loss.png', best_mat_dir+mat_name+'_test_BRDF_logMAE_loss.png')
    if not os.path.exists( best_mat_dir+mat_name+'_test_img_RMSE_loss.png'):
        shutil.copy(mat_dir+mat_name+'_test_img_RMSE_loss.png', best_mat_dir+mat_name+'_test_img_RMSE_loss.png')
    if not os.path.exists( best_mat_dir+mat_name+'_test_min_img_min_top3_loss.txt'):
        shutil.copy(mat_dir+mat_name+'_test_min_img_min_top3_loss.txt', best_mat_dir+mat_name+'_test_min_img_min_top3_loss.txt')

    min_models_name = os.listdir(mat_dir+'min')

    if not os.path.exists(output_best_rendering_target_dir):
        shutil.copytree(output_rendering_target_dir, output_best_rendering_target_dir)

    for min_model_name in min_models_name:
        epoch = min_model_name.split('_')[4]
        if not os.path.exists(output_best_rendering_output_dir+str(epoch)):
            shutil.copytree(output_rendering_output_dir+str(epoch), output_best_rendering_output_dir+str(epoch))


