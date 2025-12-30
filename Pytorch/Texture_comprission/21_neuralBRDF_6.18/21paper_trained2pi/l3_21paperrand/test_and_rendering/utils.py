import os
import numpy as np
import torch

def createFolder(dir):
    if not os.path.exists(dir):
        os.mkdir(dir)

def to0_1(img):
    img = img / 255.0
    return img

def tofu1_1(img):
    return (img - 0.5) / 0.5

def fu1_1to0_1(img):
    return img * 0.5 + 0.5

def to256(img):
    img = img * 255
    img = np.clip(img, 0, 255).astype(np.uint8)
    return img


def torch_normalize_dist(a):
    dist = torch.sqrt(torch.sum(torch.square(a), dim=-1, keepdim=True))
    return a / dist, dist

def torch_normalize(a):
    return a / (torch.sqrt(torch.sum(torch.square(a), dim=-1, keepdim=True)))

def torch_dotProduct(a, b):
    return torch.sum(torch.mul(a, b), dim=-1, keepdim=True)

def torch_checkzero(a):
    return (a == 0) * 0.00001 + (a != 0) * a

def rotateVector(vector, axis, angle):
    cosa = torch.cos(angle)
    sina = torch.sin(angle)
    out1 = torch.mul(vector, cosa)
    temp = torch_dotProduct(axis, vector)
    out2 = (1 - cosa) * temp * axis
    out3 = sina * torch_crossProduct(axis, vector)
    return out1 + out2 + out3

def torch_crossProduct(tensorA,tensorB):
    X1 = torch.unsqueeze(tensorA[0], dim=-1)
    Y1 = torch.unsqueeze(tensorA[1], dim=-1)
    Z1 = torch.unsqueeze(tensorA[2], dim=-1)
    X2 = torch.unsqueeze(tensorB[:, :, :, 0], dim=-1)
    Y2 = torch.unsqueeze(tensorB[:, :, :, 1], dim=-1)
    Z2 = torch.unsqueeze(tensorB[:, :, :, 2], dim=-1)
    dim1 = torch_dotProduct(Y1, Z2) - torch_dotProduct(Z1, Y2)
    dim2 = torch_dotProduct(Z1, X2) - torch_dotProduct(X1, Z2)
    dim3 = torch_dotProduct(X1, Y2) - torch_dotProduct(Y1, X2)
    return torch.cat((dim1, dim2, dim3), dim=-1)

# 将矩阵数值限制在（min, max）内
def clip_by_tensor(tensor, min, max):
    result = (tensor >= min) * tensor + (tensor < min) * min
    result = (result <= max) * result + (result > max) * max
    return result

# tensor中小于min的元素都变为min
def torch_maximum(min, tensor):
    result = (tensor >= min) * tensor + (tensor < min) * min
    return result

def numpy_maximum(min, x):
    result = (x >= min) * x + (x < min) *min
    return result

def untonemapx(brdf, num):
    brdf_untonemap = (num*brdf / (1 - brdf))
    return brdf_untonemap

def tonemapx(brdf, num):
    brdf_tonemap = (1*brdf) / (num+1*brdf)
    return brdf_tonemap



def exp_decy_lr(optimizer, lr, decay_rate, decay_step, step):
    lr = lr * decay_rate ** (step / decay_step)
    if lr < 0.0001:
        lr = 0.0001
    for param_group in optimizer.param_groups:
        param_group['lr'] = lr
    return lr

def rangles_to_rvectors_torch(theta_h, theta_d, phi_d):
    phi_d = normalize_phid_torch(phi_d)
    hx = torch.sin(theta_h) * 1.0
    hy = torch.sin(theta_h) * 0.0
    hz = torch.cos(theta_h)
    dx = torch.sin(theta_d) * torch.cos(phi_d)
    dy = torch.sin(theta_d) * torch.sin(phi_d)
    dz = torch.cos(theta_d)
    h_d = torch.cat((hx, hy, hz, dx, dy, dz), dim=-1)
    return h_d

def normalize_phid_torch(orig_phid):
    phid = orig_phid.clone()
    phid = torch.where(phid < 0, phid + 2 * np.pi, phid)
    phid = torch.where(phid >= 2 * np.pi, phid - 2 * np.pi, phid)
    return phid

def hd_to_hdn_dyfu11(hd):
    hx_norm = (hd[..., 0:1] - 0.5) / 0.5
    hy_norm = (hd[..., 1:2] - 0.5) / 0.5
    hz_norm = (hd[..., 2:3] - 0.5) / 0.5
    dx_norm = hd[..., 3:4]
    dy_norm = hd[..., 4:5]
    dz_norm = (hd[..., 5:6] - 0.5) / 0.5
    hd_norm = torch.cat((hx_norm, hy_norm, hz_norm, dx_norm, dy_norm, dz_norm), dim=-1)
    return hd_norm

def fu1_1to0_1(img):
    return img * 0.5 + 0.5