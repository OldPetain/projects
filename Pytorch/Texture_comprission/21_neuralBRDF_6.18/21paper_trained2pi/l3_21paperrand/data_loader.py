from torch.utils.data import Dataset
import torch
import numpy as np
import pandas as pd

import fastmerl
import common
import coords

Xvars = ['hx', 'hy', 'hz', 'dx', 'dy', 'dz']
Yvars = ['brdf_r', 'brdf_g', 'brdf_b']

class MerlDataset(Dataset):
    def __init__(self, merlPath, batchsize, max_samples, device):
        super(MerlDataset, self).__init__()

        self.bs = batchsize
        self.BRDF = fastmerl.Merl(merlPath)

        self.reflectance_train = self.__generate_nn_datasets(self.BRDF, nsamples=max_samples, pct=0.8)
        self.reflectance_test = self.__generate_nn_datasets(self.BRDF, nsamples=max_samples, pct=0.2)

        self.train_samples = torch.tensor(self.reflectance_train[Xvars].values, dtype=torch.float32, device=device)
        self.train_gt = torch.tensor(self.reflectance_train[Yvars].values, dtype=torch.float32, device=device)

        self.test_samples = torch.tensor(self.reflectance_test[Xvars].values, dtype=torch.float32, device=device)
        self.test_gt = torch.tensor(self.reflectance_test[Yvars].values, dtype=torch.float32, device=device)

    def __len__(self):
        return self.train_samples.shape[0]

    def get_trainbatch(self, idx):
        return self.train_samples[idx:idx + self.bs, :], self.train_gt[idx:idx + self.bs, :]

    def get_testbatch(self, idx):
        return self.test_samples[idx:idx + self.bs, :], self.test_gt[idx:idx + self.bs, :]

    def shuffle(self):
        r = torch.randperm(self.train_samples.shape[0])
        self.train_samples = self.train_samples[r, :]
        self.train_gt = self.train_gt[r, :]

    def __getitem__(self, idx):
        pass

    def __generate_nn_datasets(self, brdf, nsamples=800000, pct=0.8):
        rangles = np.random.uniform([0, 0, 0], [np.pi / 2., np.pi / 2., 2 * np.pi], [int(nsamples * pct), 3]).T
        rangles[2] = common.normalize_phid(rangles[2])

        rvectors = coords.rangles_to_rvectors(*rangles)
        brdf_vals = self.__brdf_values(rvectors, brdf=brdf)

        df = pd.DataFrame(np.concatenate([rvectors.T, brdf_vals], axis=1), columns=[*Xvars, *Yvars])
        df = df[(df.T != 0).any()]
        df = df.drop(df[df['brdf_r'] < 0].index)
        return df

    def __brdf_values(self, rvectors, brdf=None, model=None):
        if brdf is not None:
            rangles = coords.rvectors_to_rangles(*rvectors)
            brdf_arr = brdf.eval_interp(*rangles).T
        elif model is not None:
            # brdf_arr = model.predict(rvectors.T)        # nnModule has no .predict
            raise RuntimeError("Should not have entered that branch at all from the original code")
        else:
            raise NotImplementedError("Something went really wrong.")
        brdf_arr *= common.mask_from_array(rvectors.T).reshape(-1, 1)
        return brdf_arr

    def shuffle_data(self):
        r = torch.randperm(self.train_samples.shape[0])
        self.train_samples = self.train_samples[r, :]
        self.train_gt = self.train_gt[r, :]

