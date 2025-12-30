<!--
Created by Mitsubishi Electric Research Laboratories (MERL), 2003, 2007, 2023

SPDX-License-Identifier: CC-BY-SA-4.0
-->

# BRDF Database

## Introduction
The MERL BRDF database contains reflectance functions of 100 different materials. Each reflectance function is stored as a densely measured Bidirectional Reflectance Distribution Function (BRDF).

Sample code to read the data is included with the database (`readBRDF.cpp`). Note that parameterization of theta-half has changed.

## Dataset Organization

The size of the unzipped dataset is ~3.25GB. Folder `brdfs` contains the data. Folder `code` contains sample code.

## Citation

If you use the BRDF dataset in your research, please cite our contribution:

```BibTex
@article {Matusik:2003,
	author = "Wojciech Matusik and Hanspeter Pfister and Matt Brand and Leonard McMillan",
	title = "A Data-Driven Reflectance Model",
	journal = "ACM Transactions on Graphics",
	year = "2003",
	month = jul,
	volume = "22",
	number = "3",
	pages = "759-769"
}
```

## License

The BRDF dataset is released under [CC-BY-SA-4.0](https://creativecommons.org/licenses/by-sa/4.0/) license.

All data:

```
Created by Mitsubishi Electric Research Laboratories (MERL), 2003, 2007, 2023

SPDX-License-Identifier: CC-BY-SA-4.0
```



# 进入示例代码目录
cd Texture_compression/BRDFDatabase/code

# 编译（示例）
g++ -O2 BRDFRead.cpp -o brdf_reader

# 运行（将输出重定向到文本文件）
./brdf_reader ../brdfs/aluminium.binary > aluminium.txt
./brdf_reader ../brdfs/blue-acrylic.binary > blue-acrylic.txt

# 输出说明：每行三个浮点数 (R G B)，对应程序在一个固定角度网格 (16 x 64 x 16 x 64) 上采样得到的 BRDF 值。

# 可视化
python3 brdf_txt_viewer.py
python3 brdf_merl_viewer.py  
（source ~/Pytorch/venv38/bin/activate）

# Mag 表示采样得到的 RGB 三通道值向量的模长（Euclid 长度），等于 sqrt(R^2 + G^2 + B^2)。它反映了原始三通道值的总体强度（未做 tone-mapping 前的能量等级）
# Dir 表示该 RGB 向量的归一化方向（单位向量）：(R/Mag, G/Mag, B/Mag)。它反映的是颜色的“色度/配色比例”——即 R、G、B 三个通道相对贡献，不包含亮度信息