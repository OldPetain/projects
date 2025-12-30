import torch
import math

from test_and_rendering import utils

scalefi = 180
scaletheta = 90

class RenderImages:
    def __init__(self, target_brdf, size, lamp, res, test_li_pos, test_cam_pos, device):
        self.target_brdf = target_brdf
        self.size = size
        self.lamp = lamp
        self.res = res
        self.test_li_pos = test_li_pos
        self.test_cam_pos = test_cam_pos
        self.device = device

    def test_compute_half_diff(self):
        surface_coord = self.__surface_array()
        wi, wo, wi_norm, wo_norm, dist = self.__generate_wiwo(surface_coord)
        thetah, thetad, fid_fupipi = self.__std_coords_2_half_diff_coords(wi_norm, wo_norm)

        return thetah, thetad, fid_fupipi, dist, wi_norm

    def test_neuralBRDF_render_images(self, brdf, dist, wi_norm):
        normal_v = torch.tensor([0,0,1]).to(self.device)
        n_dot_wi = utils.torch_dotProduct(normal_v, wi_norm)

        output_result = brdf* n_dot_wi * self.lamp / (dist**2)
        return output_result

    def test_render_images(self, thetah, thetad, phid, dist, wi_norm):
        target_brdf, n_dot_wi = self.__look_up_render_images(thetah, thetad, phid, wi_norm)
        target_result = target_brdf * n_dot_wi * self.lamp / (dist ** 2)
        target_result = utils.torch_maximum(0.0001, target_result)
        return target_result


    def __surface_array(self):
        x_coord = torch.linspace(-1, 1, self.res, device=self.device) * (self.size/2)
        x_coord = x_coord.unsqueeze(0).expand((self.res, self.res)).unsqueeze(-1)
        y_coord = -1 * torch.transpose(x_coord, 0, 1)
        surface_coord = torch.cat((x_coord, y_coord, torch.zeros_like(x_coord)), dim=-1)
        surface_coord = surface_coord.unsqueeze(0)
        return surface_coord

    def __generate_wiwo(self, surface_coord):
        test_li_pos = self.test_li_pos.unsqueeze(-2).unsqueeze(-2)
        test_cam_pos = self.test_cam_pos.unsqueeze(-2).unsqueeze(-2)

        wi = test_li_pos - surface_coord
        wo = test_cam_pos - surface_coord
        wi_norm, dist = utils.torch_normalize_dist(wi)
        wo_norm = utils.torch_normalize(wo)
        return wi, wo, wi_norm, wo_norm, dist

    def __std_coords_2_half_diff_coords(self, wi_norm, wo_norm):
        h_norm = utils.torch_normalize((wi_norm+wo_norm)/2)

        normal_v = torch.tensor([0,0,1]).to(self.device)
        costhetah = utils.torch_dotProduct(h_norm, normal_v)
        thetah = torch.acos(costhetah) # 弧度制

        h_x = torch.unsqueeze(h_norm[...,0], dim=-1)
        h_y = torch.unsqueeze(h_norm[...,1], dim=-1)
        h_x = utils.torch_checkzero(h_x)
        fih = torch.atan2(h_y, h_x)

        normal_temp1 = torch.tensor([0,0,1]).to(self.device)
        normal_temp2 = torch.tensor([0,1,0]).to(self.device)
        inverse_thetah = -1 * thetah
        inverse_fih = -1 * fih

        temp = utils.rotateVector(wi_norm, normal_temp1, inverse_fih)
        diff = utils.rotateVector(temp, normal_temp2, inverse_thetah)

        costhetad = utils.torch_dotProduct(normal_v, diff)
        # 防止在1附近acos出现nan  ??
        costhetad_new = utils.clip_by_tensor(costhetad, -0.9999, 0.9999)
        thetad = torch.acos(costhetad_new)

        d_x = torch.unsqueeze(diff[...,0], dim=-1)
        d_y = torch.unsqueeze(diff[..., 1], dim=-1)
        d_x = utils.torch_checkzero(d_x)
        fid_fupipi = torch.atan2(d_y, d_x)


        return thetah, thetad, fid_fupipi

    def __look_up_render_images(self, thetah, thetad, fid, wi_norm):
        # 弧度转化为角度
        index_fid = torch.div(fid, math.pi) * scalefi
        index_thetad = torch.div(thetad, (math.pi / 2)) * scaletheta
        # index_thetah = torch.div(thetah, (math.pi / 2)) * scaletheta
        # 为什么而是用sqrt?应该是有利于在垂直入射附近的角度采样多一些,但为什么sqrt能起这样的作用呢？
        index_thetah = torch.div(thetah, (math.pi / 2))
        index_thetah = torch.sqrt(index_thetah)
        index_thetah = index_thetah * scaletheta

        # rate0:x-x0, rate1:x1-x
        rate0_fid, rate1_fid = self.__computeRate(index_fid)
        rate0_thetad, rate1_thetad = self.__computeRate(index_thetad)
        rate0_thetah, rate1_thetah = self.__computeRate(index_thetah)

        rates_fid = [rate0_fid, rate1_fid]
        rates_thetad = [rate0_thetad, rate1_thetad]
        rates_thetah = [rate0_thetah, rate1_thetah]

        index1_fid, index0_fid = self.__generateCeilFloor(index_fid)
        index1_thetad, index0_thetad = self.__generateCeilFloor(index_thetad)
        index1_thetah, index0_thetah = self.__generateCeilFloor(index_thetah)

        indexs_fid = [index1_fid, index0_fid]
        indexs_thetad = [index1_thetad, index0_thetad]
        indexs_thetah = [index1_thetah, index0_thetah]

        i = 0
        for rate_fid, index_fid in zip(rates_fid, indexs_fid):
            for rate_thetad, index_thetad in zip(rates_thetad, indexs_thetad):
                for rate_thetah, index_thetah in zip(rates_thetah, indexs_thetah):
                    if i == 0:
                        target_brdf = self.__interpolate_check(
                            torch.mul(torch.mul(rate_fid, rate_thetad), rate_thetah),
                            index_fid, index_thetad, index_thetah)
                    else:
                        target_brdf_add = self.__interpolate_check(
                            torch.mul(torch.mul(rate_fid, rate_thetad), rate_thetah),
                            index_fid, index_thetad, index_thetah)
                        target_brdf = target_brdf + target_brdf_add
                    i = i + 1
                    # utils.test_out(BRDF)

        normal_v = torch.tensor([0, 0, 1]).to(self.device)
        n_dot_wi = utils.torch_dotProduct(normal_v, wi_norm)
        return target_brdf, n_dot_wi

    def __computeRate(self, angle):
        rate0 = angle - torch.floor(angle)
        rate1 = 1.0 - rate0
        return rate0, rate1

    def __generateCeilFloor(self, index):
        return torch.ceil(index), torch.floor(index)

    def __interpolate_check(self, rate, index_fid, index_thetad, index_thetah):
        # 如果index_fid取180时，不知道原因？？减1的原因是index_thetad和index_thetah取90时，水平射向表面，不可能有反射光，
        index_fid = utils.clip_by_tensor(index_fid, 0, scalefi - 1).to(torch.int64)
        index_thetad = utils.clip_by_tensor(index_thetad, 0, scaletheta - 1).to(torch.int64)
        index_thetah = utils.clip_by_tensor(index_thetah, 0, scaletheta - 1).to(torch.int64)

        index = index_fid + index_thetad*scalefi + index_thetah*scalefi*scaletheta
        index = index.to(torch.int64)

        target_brdf_check = torch.squeeze(self.target_brdf[index], dim=-2)
        target_brdf_check_rate = utils.torch_maximum(0, target_brdf_check) * rate

        return target_brdf_check_rate



