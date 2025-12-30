import os
import numpy as np
import torch



output_dir = './output/out_21paper_h21l3_train2pi_batch512_lr5e4/blue-acrylic/split_min1_loss_model_000096/'
model_dir = './output/out_21paper_h21l3_train2pi_batch512_lr5e4/blue-acrylic/min/blue-acrylic_min1_loss_model_000096_0.016645.pth'


parameters = torch.load(model_dir)
# print(parameters)

for param_name in parameters:
    weight = parameters[param_name]
    segs = param_name.split('.')
    if segs[-1] == 'weight':
        param_name = segs[0]
    else:
        param_name = segs[0].replace('fc', 'b')

    filename = 'blue-acrylic_{}.npy'.format(param_name)
    filepath = os.path.join(output_dir, filename)
    curr_weight = weight.cpu().numpy().T
    np.save(filepath,curr_weight)





# for el in model.named_parameters():
#     param_name = el[0]   # either fc1.bias or fc1.weight
#     weights = el[1]
#     segs = param_name.split('.')
#     if segs[-1] == 'weight':
#         param_name = segs[0]
#     else:
#         param_name = segs[0].replace('fc', 'b')
#
#     filename = '_{}.npy'.format(param_name)
#     filepath = os.path.join(outpath, filename)
#     curr_weight = weights.detach().cpu().numpy().T  # transpose bc mitsuba code was developed for TF convention
#     np.save(filepath, curr_weight)