import numpy as np
import torch
import os

from test_and_rendering import find_value


red_scale = 1/1500.0
green_scale = 1.15/1500.0
blue_scale = 1.66/1500.0

class DataSet:
    def __init__(self, data_dir, device, test_angles_dir, testpd_pi_or_2pi):
        self.data_dir = data_dir
        self.device = device
        self.test_angles_dir = test_angles_dir
        self.testpd_pi_or_2pi = testpd_pi_or_2pi

    def get_test_data_hd(self):
        brdf_values = self.__read_merl()
        self.test_valid_input_hd_t, self.test_valid_brdf_sample_t = self.__test_get_hd_thtdpd_brdf(brdf_values)
        return brdf_values

    def test_get_merl(self):
        return self.__read_merl()


    def get_test_sample_pd2pi(self):
        return self.test_valid_input_hd_t, self.test_valid_brdf_sample_t

    def __read_merl(self):
        try:
            f = open(self.data_dir+'.binary', "rb")
            dims = np.fromfile(f, np.int32, 3)
            values = np.fromfile(f, np.float64, -1)
            f.close()

        except IOError:
            print("Cannot read file:", self.data_dir)
            return

        brdf_values = np.reshape(values, [3, dims[0]*dims[1]*dims[2]])
        brdf_values_c1 = np.expand_dims(brdf_values[0, ...] * red_scale, axis=-1)
        brdf_values_c2 = np.expand_dims(brdf_values[1, ...] * green_scale, axis=-1)
        brdf_values_c3 = np.expand_dims(brdf_values[2, ...] * blue_scale, axis=-1)
        # (thetah*thetad*phid, rgb)=(90*90*180, 3)
        brdf_values = np.concatenate(( brdf_values_c1,  brdf_values_c2,  brdf_values_c3), axis=1)
        return brdf_values

    def __read_angles(self):
        angles_name = os.listdir(self.test_angles_dir)
        input_th_td_pd = np.zeros((400000, 3))
        i = 0
        for angles_name in angles_name:
            test_angles_file_dir = self.test_angles_dir + angles_name
            input_angle = np.loadtxt(test_angles_file_dir, delimiter=',')
            n = input_angle.shape[0]
            input_th_td_pd[i*n:(i+1)*n, :] = input_angle
            i = i + 1
        return input_th_td_pd

    def __lookup_sample_brdf(self, brdf_values_t, angles_sample_t):
        look_up_sample_object = find_value.LookUpSample(brdf_values_t, angles_sample_t)
        brdf_sample_tensor = look_up_sample_object.look_up_sample()
        return brdf_sample_tensor


    def __test_get_hd_thtdpd_brdf(self, brdf_values):
        th_td_pd = self.__read_angles()
        if self.testpd_pi_or_2pi == '2pi':
            '''
            th(0,π/2), td(0,π/2)， pd(0，2π) ==> hd ==>  th(0,π/2), td(0,π/2)， pd(-π，π)
            hd==>hd(-1,1)  
            '''
            input_hd = self.__rangles_to_rvectors(th_td_pd[:,0], th_td_pd[:,1],th_td_pd[:,2])
            input_th_td_pd = self.__rvectors_to_rangles(input_hd) # th(0,π/2), td(0,π/2)， pd(-π，π)

        brdf_values_t = torch.from_numpy(brdf_values).to(self.device).type(torch.float32)
        input_th_td_pd_t = torch.from_numpy(input_th_td_pd).to(self.device).type(torch.float32)  # 用于查找RGB
        input_hd_t = torch.from_numpy(input_hd).to(self.device).type(torch.float32)  # 用于net的输入

        brdf_sample_t = self.__lookup_sample_brdf(brdf_values_t, input_th_td_pd_t)
        # 计算mask
        brdf_sample_t[:, 0][brdf_sample_t[:, 0] < 0] = -1
        brdf_sample_t[:, 0][brdf_sample_t[:, 1] < 0] = -1
        brdf_sample_t[:, 0][brdf_sample_t[:, 2] < 0] = -1
        mask = (brdf_sample_t[:, 0] != -1)

        # 删除无效值
        valid_hd_t = input_hd_t[mask]
        valid_brdf_sample_t = brdf_sample_t[mask]

        return valid_hd_t, valid_brdf_sample_t

    def __normalize_phid(self, orig_pd):
        pd = orig_pd.copy()
        pd = np.where(pd < 0, pd + 2*np.pi, pd)
        pd = np.where(pd >= 2*np.pi, pd - 2*np.pi, pd)
        return pd

    def __rvectors_to_rangles(self, hd):
        hx = hd[..., 0]
        hy = hd[..., 1]
        hz = hd[..., 2]
        dx = hd[..., 3]
        dy = hd[..., 4]
        dz = hd[..., 5]
        th = np.arctan2(np.sqrt(hx**2+hy**2), hz)
        td = np.arctan2(np.sqrt(dx**2+dy**2), dz)
        pd = np.arctan2(dy, dx)
        return np.array([th, td, pd]).T

    # assumes phi_h=0 and both norms=1
    def __rangles_to_rvectors(self, theta_h, theta_d, phi_d):
        hx = np.sin(theta_h) * np.cos(0.0)
        hy = np.sin(theta_h) * np.sin(0.0)
        hz = np.cos(theta_h)
        dx = np.sin(theta_d) * np.cos(phi_d)
        dy = np.sin(theta_d) * np.sin(phi_d)
        dz = np.cos(theta_d)
        return np.array([hx, hy, hz, dx, dy, dz]).T

    def __hd_to_fu11hd_pd2pi(self, hd):
        hx_norm = (hd[..., 0:1] - 0.5) / 0.5
        hy_norm = (hd[..., 1:2] - 0.5) / 0.5
        hz_norm = (hd[..., 2:3] - 0.5) / 0.5
        dx_norm = hd[..., 3:4]
        dy_norm = hd[..., 4:5]
        dz_norm = (hd[..., 5:6] - 0.5) / 0.5
        hd_norm = np.concatenate((hx_norm, hy_norm, hz_norm, dx_norm, dy_norm, dz_norm), axis=-1)
        return hd_norm








