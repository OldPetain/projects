// Copyright 2005 Mitsubishi Electric Research Laboratories All Rights Reserved.

// Permission to use, copy and modify this software and its documentation without
// fee for educational, research and non-profit purposes, is hereby granted, provided
// that the above copyright notice and the following three paragraphs appear in all copies.

// To request permission to incorporate this software into commercial products contact:
// Vice President of Marketing and Business Development;
// Mitsubishi Electric Research Laboratories (MERL), 201 Broadway, Cambridge, MA 02139 or 
// <license@merl.com>.

// IN NO EVENT SHALL MERL BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL,
// OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
// ITS DOCUMENTATION, EVEN IF MERL HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

// MERL SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED
// HEREUNDER IS ON AN "AS IS" BASIS, AND MERL HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
// UPDATES, ENHANCEMENTS OR MODIFICATIONS.


#include "stdlib.h"
#include "math.h"
#include <cstdio>

#define BRDF_SAMPLING_RES_THETA_H       90
#define BRDF_SAMPLING_RES_THETA_D       90
#define BRDF_SAMPLING_RES_PHI_D         360

#define RED_SCALE (1.0/1500.0)
#define GREEN_SCALE (1.15/1500.0)
#define BLUE_SCALE (1.66/1500.0)
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// 计算两个向量的叉乘
void cross_product (double* v1, double* v2, double* out)
{
	out[0] = v1[1]*v2[2] - v1[2]*v2[1];
	out[1] = v1[2]*v2[0] - v1[0]*v2[2];
	out[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

// 向量归一化
void normalize(double* v)
{
	// normalize
	double len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	v[0] = v[0] / len;
	v[1] = v[1] / len;
	v[2] = v[2] / len;
}

// 绕任意轴旋转向量
void rotate_vector(double* vector, double* axis, double angle, double* out)
{
	double temp;
	double cross[3];
	double cos_ang = cos(angle);
	double sin_ang = sin(angle);

	out[0] = vector[0] * cos_ang;
	out[1] = vector[1] * cos_ang;
	out[2] = vector[2] * cos_ang;

	temp = axis[0]*vector[0]+axis[1]*vector[1]+axis[2]*vector[2];
	temp = temp*(1.0-cos_ang);

	out[0] += axis[0] * temp;
	out[1] += axis[1] * temp;
	out[2] += axis[2] * temp;

	cross_product (axis,vector,cross);
	
	out[0] += cross[0] * sin_ang;
	out[1] += cross[1] * sin_ang;
	out[2] += cross[2] * sin_ang;
}

// 将标准入/出射角坐标转换为半向量/差向量坐标系
void std_coords_to_half_diff_coords(double theta_in, double fi_in, double theta_out, double fi_out,
								double& theta_half,double& fi_half,double& theta_diff,double& fi_diff )
{

	// 计算入射向量
	double in_vec_z = cos(theta_in);
	double proj_in_vec = sin(theta_in);
	double in_vec_x = proj_in_vec*cos(fi_in);
	double in_vec_y = proj_in_vec*sin(fi_in);
	double in[3]= {in_vec_x,in_vec_y,in_vec_z};
	normalize(in);


	// 计算出射向量
	double out_vec_z = cos(theta_out);
	double proj_out_vec = sin(theta_out);
	double out_vec_x = proj_out_vec*cos(fi_out);
	double out_vec_y = proj_out_vec*sin(fi_out);
	double out[3]= {out_vec_x,out_vec_y,out_vec_z};
	normalize(out);


	// 计算半向量
	double half_x = (in_vec_x + out_vec_x)/2.0f;
	double half_y = (in_vec_y + out_vec_y)/2.0f;
	double half_z = (in_vec_z + out_vec_z)/2.0f;
	double half[3] = {half_x,half_y,half_z};
	normalize(half);

	// 计算 theta_half 和 fi_half
	theta_half = acos(half[2]);
	fi_half = atan2(half[1], half[0]);


	double bi_normal[3] = {0.0, 1.0, 0.0};
	double normal[3] = { 0.0, 0.0, 1.0 };
	double temp[3];
	double diff[3];

	// 计算差向量
	rotate_vector(in, normal , -fi_half, temp);
	rotate_vector(temp, bi_normal, -theta_half, diff);
	
	// 计算 theta_diff 和 fi_diff	
	theta_diff = acos(diff[2]);
	fi_diff = atan2(diff[1], diff[0]);

}

// 查找 theta_half 的索引
// 注意：这是一个非线性映射
// 输入范围:  [0 .. pi/2]
// 输出范围:  [0 .. 89]
inline int theta_half_index(double theta_half)
{
	if (theta_half <= 0.0)
		return 0;
	double theta_half_deg = ((theta_half / (M_PI/2.0))*BRDF_SAMPLING_RES_THETA_H);
	double temp = theta_half_deg*BRDF_SAMPLING_RES_THETA_H;
	temp = sqrt(temp);
	int ret_val = (int)temp;
	if (ret_val < 0) ret_val = 0;
	if (ret_val >= BRDF_SAMPLING_RES_THETA_H)
		ret_val = BRDF_SAMPLING_RES_THETA_H-1;
	return ret_val;
}

// 查找 theta_diff 的索引
// 输入范围:  [0 .. pi/2]
// 输出范围:  [0 .. 89]
inline int theta_diff_index(double theta_diff)
{
	int tmp = int(theta_diff / (M_PI * 0.5) * BRDF_SAMPLING_RES_THETA_D);
	if (tmp < 0)
		return 0;
	else if (tmp < BRDF_SAMPLING_RES_THETA_D - 1)
		return tmp;
	else
		return BRDF_SAMPLING_RES_THETA_D - 1;
}

// 查找 phi_diff 的索引
inline int phi_diff_index(double phi_diff)
{
	// 由于互易性，BRDF 在 phi_diff -> phi_diff + M_PI 下不变
	if (phi_diff < 0.0)
		phi_diff += M_PI;

	// 输入: phi_diff 在 [0 .. pi]
	// 输出: tmp 在 [0 .. 179]
	int tmp = int(phi_diff / M_PI * BRDF_SAMPLING_RES_PHI_D / 2);
	if (tmp < 0)	
		return 0;
	else if (tmp < BRDF_SAMPLING_RES_PHI_D / 2 - 1)
		return tmp;
	else
		return BRDF_SAMPLING_RES_PHI_D / 2 - 1;
}

// 给定一对入射/出射角，查找并返回 BRDF 值。
void lookup_brdf_val(double* brdf, double theta_in, double fi_in,
			  double theta_out, double fi_out, 
			  double& red_val,double& green_val,double& blue_val)
{
	// 转换为半角/差角坐标
	double theta_half, fi_half, theta_diff, fi_diff;
	
	std_coords_to_half_diff_coords(theta_in, fi_in, theta_out, fi_out,
		       theta_half, fi_half, theta_diff, fi_diff);


	// 计算数组索引
	// 注意：这里忽略了 phi_half，因为假设 BRDF 在各向同性情况下对 phi_half 不变
	int ind = phi_diff_index(fi_diff) +
		  theta_diff_index(theta_diff) * BRDF_SAMPLING_RES_PHI_D / 2 +
		  theta_half_index(theta_half) * BRDF_SAMPLING_RES_PHI_D / 2 *
					         BRDF_SAMPLING_RES_THETA_D;

	red_val = brdf[ind] * RED_SCALE;
	green_val = brdf[ind + BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D/2] * GREEN_SCALE;
	blue_val = brdf[ind + BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D] * BLUE_SCALE;

	
	if (red_val < 0.0 || green_val < 0.0 || blue_val < 0.0)
		fprintf(stderr, "Below horizon.\n");

}

// 读取 BRDF 数据
bool read_brdf(const char *filename, double* &brdf)
{
	FILE *f = fopen(filename, "rb");
	if (!f)
		return false;

	int dims[3];
	fread(dims, sizeof(int), 3, f);
	int n = dims[0] * dims[1] * dims[2];
	if (n != BRDF_SAMPLING_RES_THETA_H *
		 BRDF_SAMPLING_RES_THETA_D *
		 BRDF_SAMPLING_RES_PHI_D / 2) 
	{
		// 尺寸不匹配
		fprintf(stderr, "Dimensions don't match\n");
		fclose(f);
		return false;
	}

	brdf = (double*) malloc (sizeof(double)*3*n);
	fread(brdf, sizeof(double), 3*n, f);

	fclose(f);
	return true;
}

int main(int argc, char *argv[])
{
	const char *filename = argv[1];
	double* brdf;

	// 读取 BRDF 数据（主流程）
	if (!read_brdf(filename, brdf)) 
	{
		// 读取失败
		fprintf(stderr, "Error reading %s\n", filename);
		exit(1);
	}

	// 将 BRDF 值按 16x64x16x64 的网格打印输出为文本
	const int n = 16;
	for (int i = 0; i < n; i++) 
	{
	    double theta_in = i * 0.5 * M_PI / n;
	    for (int j = 0; j < 4*n; j++) 
		{
			double phi_in = j * 2.0 * M_PI / (4*n);
			for (int k = 0; k < n; k++) 
			{
				double theta_out = k * 0.5 * M_PI / n;
				for (int l = 0; l < 4*n; l++) 
				{
					double phi_out = l * 2.0 * M_PI / (4*n);
					double red,green,blue;
					lookup_brdf_val(brdf, theta_in, phi_in, theta_out, phi_out, red, green, blue);
					printf("%f %f %f\n", (float)red, (float)green, (float)blue);
				}
			}
	    }
	}
	return 0;
}


