'''
21neural BRDF中的find_value中：phid180变为0, thetah和thetah89.x变为88与89的加权求和
我的find_value中：phid180变为phid179, thetah和thetah89.x变为89
'''
import numpy as np
import math
import torch


SCALE_PHI= 180
SCALE_THETA = 90

class LookUpSample:
    def __init__(self, brdf_values, angles):
        self.brdf_values = brdf_values
        self.angles = angles

    def look_up_sample(self):
        thetah = self.angles[...,:1]
        thetad = self.angles[...,1:2]
        phid = self.angles[...,2:3]
        # (-pi, pi)--> (0, pi)
        phid = (phid < 0.0) * (phid + math.pi) + (phid > 0.0) * phid
        index_phid = torch.div(phid, math.pi) * SCALE_PHI
        index_thetad = torch.div(thetad, math.pi/2) * SCALE_THETA

        index_thetah = torch.div(thetah, math.pi/2)
        index_thetah = torch.sqrt(index_thetah)
        index_thetah = index_thetah * SCALE_THETA

        index1_thetah, index0_thetah = self.__get_ceil_floor(index_thetah)
        index1_thetad, index0_thetad = self.__get_ceil_floor(index_thetad)
        index1_phid, index0_phid = self.__get_ceil_floor(index_phid)

        rate0_thetah, rate1_thetah = self.__get_rate(index_thetah)
        rate0_thetad, rate1_thetad = self.__get_rate(index_thetad)
        rate0_phid, rate1_phid = self.__get_rate(index_phid)

        indexs_thetah = [index1_thetah, index0_thetah]
        indexs_thetad = [index1_thetad, index0_thetad]
        indexs_phid = [index1_phid, index0_phid]

        rates_thetah = [rate0_thetah, rate1_thetah]
        rates_thetad = [rate0_thetad, rate1_thetad]
        rates_phid = [rate0_phid, rate1_phid]

        i = 0
        for rate_phid, index_phid in zip(rates_phid, indexs_phid):
            for rate_thetad, index_thetad in zip(rates_thetad, indexs_thetad):
                for rate_thetah, index_thetah in zip(rates_thetah, indexs_thetah):
                    if i == 0:
                        value = self.__interpolate_value(rate_phid*rate_thetad*rate_thetah, index_phid, index_thetad, index_thetah)
                    else:
                        value_add = self.__interpolate_value(rate_phid*rate_thetad*rate_thetah, index_phid, index_thetad, index_thetah)
                        value = value + value_add
                    i = i + 1

        return value

    def look_up_sample_sqrtthetah(self):
        thetah_r = self.angles[...,:1]
        thetad_r = self.angles[...,1:2]
        phid_r = self.angles[...,2:3]

        # index_phid = torch.div(phid, math.pi) * SCALE_PHI
        # index_thetad = torch.div(thetad, math.pi/2) * SCALE_THETA
        #
        # index_thetah = torch.div(thetah, math.pi/2)
        # index_thetah = torch.sqrt(index_thetah)
        # index_thetah = index_thetah * SCALE_THETA

        index_phid = phid_r * SCALE_PHI
        index_thetad = thetad_r* SCALE_THETA
        index_thetah = thetah_r * SCALE_THETA

        index1_thetah, index0_thetah = self.__get_ceil_floor(index_thetah)
        index1_thetad, index0_thetad = self.__get_ceil_floor(index_thetad)
        index1_phid, index0_phid = self.__get_ceil_floor(index_phid)

        rate0_thetah, rate1_thetah = self.__get_rate(index_thetah)
        rate0_thetad, rate1_thetad = self.__get_rate(index_thetad)
        rate0_phid, rate1_phid = self.__get_rate(index_phid)

        indexs_thetah = [index1_thetah, index0_thetah]
        indexs_thetad = [index1_thetad, index0_thetad]
        indexs_phid = [index1_phid, index0_phid]

        rates_thetah = [rate0_thetah, rate1_thetah]
        rates_thetad = [rate0_thetad, rate1_thetad]
        rates_phid = [rate0_phid, rate1_phid]

        i = 0
        for rate_phid, index_phid in zip(rates_phid, indexs_phid):
            for rate_thetad, index_thetad in zip(rates_thetad, indexs_thetad):
                for rate_thetah, index_thetah in zip(rates_thetah, indexs_thetah):
                    if i == 0:
                        value = self.__interpolate_value(rate_phid*rate_thetad*rate_thetah, index_phid, index_thetad, index_thetah)
                    else:
                        value_add = self.__interpolate_value(rate_phid*rate_thetad*rate_thetah, index_phid, index_thetad, index_thetah)
                        value = value + value_add
                    i = i + 1

        return value


    def __get_ceil_floor(self, index):
        ceil = torch.ceil(index)
        floor = torch.floor(index)
        return ceil, floor

    def __get_rate(self, index):
        rate0 = index - torch.floor(index)
        rate1 = 1.0 - rate0
        return rate0, rate1

    def __interpolate_value(self, rate, index_phid, index_thetad, index_thetah):
        index_phid = torch.clip(index_phid, 0, SCALE_PHI-1).to(torch.int64)
        index_thetad = torch.clip(index_thetad, 0, SCALE_THETA-1).to(torch.int64)
        index_thetah = torch.clip(index_thetah, 0, SCALE_THETA-1).to(torch.int64)
        index = index_phid + index_thetad * SCALE_PHI + index_thetah * SCALE_PHI * SCALE_THETA

        brdf_value = torch.squeeze(self.brdf_values[index], dim=-2)
        brdf_value = brdf_value * rate
        return brdf_value

