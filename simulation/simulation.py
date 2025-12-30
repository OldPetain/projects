import numpy as np

def single_slit_diffraction(theta, wavelength, slit_width):
    """
    计算单缝夫琅禾费衍射光强归一化分布
    theta: 入射角数组（弧度）
    wavelength: 波长（米）
    slit_width: 单缝宽度（米）
    """
    beta = (np.pi * slit_width / wavelength) * np.sin(theta)
    # 避免除零错误
    beta = np.where(beta == 0, 1e-10, beta)
    intensity = (np.sin(beta)/beta)**2
    return intensity

def multi_slit_interference(theta, wavelength, slit_distance, num_slits):
    """
    多缝干涉光强分布归一化
    theta: 入射角数组（弧度）
    wavelength: 波长（米）
    slit_distance: 缝间距（米）
    num_slits: 通光缝数量
    """
    alpha = (np.pi * slit_distance / wavelength) * np.sin(theta)
    # 避免除零
    alpha = np.where(alpha == 0, 1e-10, alpha)
    numerator = np.sin(num_slits * alpha)
    denominator = np.sin(alpha)
    intensity = (numerator / denominator)**2 / num_slits**2
    return intensity

def total_intensity(theta, wavelength, slit_width, slit_distance, num_slits):
    """
    综合单缝衍射包络与多缝干涉光强计算
    """
    single = single_slit_diffraction(theta, wavelength, slit_width)
    multi = multi_slit_interference(theta, wavelength, slit_distance, num_slits)
    return single * multi
