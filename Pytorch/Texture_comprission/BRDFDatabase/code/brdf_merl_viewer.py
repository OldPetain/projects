#!/usr/bin/env python3
"""
MERL BRDF 插值查看器（仅二进制 .binary + rvectors）

用法:
    python3 brdf_merl_viewer.py

说明:
    只加载 MERL 二进制 BRDF 文件 (.binary)，接收 rvectors: hx hy hz  dx dy dz，
    使用三线性插值获取 BRDF 三通道值，并通过曝光/简单 tone-mapping 与 gamma 校正显示预览颜色。
"""

import tkinter as tk
from tkinter import filedialog, messagebox
import math
import os
import numpy as np

# MERL 表格分辨率与缩放因子（与 C++ 实现一致）
BRDF_SAMPLING_RES_THETA_H = 90
BRDF_SAMPLING_RES_THETA_D = 90
BRDF_SAMPLING_RES_PHI_D = 360

RED_SCALE = 1.0/1500.0
GREEN_SCALE = 1.15/1500.0
BLUE_SCALE = 1.66/1500.0


class BRDFData:
    def __init__(self):
        self.loaded_merl = False

    def load_binary_merl(self, path):
        with open(path, 'rb') as f:
            dims = np.fromfile(f, dtype=np.int32, count=3)
            if dims.size != 3:
                raise ValueError('无法读取 BRDF 二进制文件的 dims')
            n = int(dims[0] * dims[1] * dims[2])
            data = np.fromfile(f, dtype=np.float64, count=3*n)
            if data.size != 3*n:
                raise ValueError(f'读取 BRDF 数据失败: 期望 {3*n} 个 double，实际 {data.size}')
        self.merl_r = data[0:n].astype(np.float64)
        self.merl_g = data[n:2*n].astype(np.float64)
        self.merl_b = data[2*n:3*n].astype(np.float64)
        self.merl_n = n
        self.merl_dims = (int(dims[0]), int(dims[1]), int(dims[2]))
        self.loaded_merl = True

    def merl_trilinear_sample(self, theta_half, theta_diff, phi_diff):
        if not self.loaded_merl:
            raise RuntimeError('尚未加载 MERL 二进制 BRDF 文件')
        if phi_diff < 0.0:
            phi_diff += math.pi

        H = BRDF_SAMPLING_RES_THETA_H
        D = BRDF_SAMPLING_RES_THETA_D
        P = BRDF_SAMPLING_RES_PHI_D // 2

        ih = (H - 1) * math.sqrt(max(0.0, theta_half) / (0.5 * math.pi))
        idd = (D - 1) * (max(0.0, theta_diff) / (0.5 * math.pi))
        ip = (P - 1) * (phi_diff / math.pi)

        ih = max(0.0, min(H - 1 - 1e-8, ih))
        idd = max(0.0, min(D - 1 - 1e-8, idd))
        ip = max(0.0, min(P - 1 - 1e-8, ip))

        ih0 = int(math.floor(ih)); ih1 = min(ih0 + 1, H-1)
        id0 = int(math.floor(idd)); id1 = min(id0 + 1, D-1)
        ip0 = int(math.floor(ip)); ip1 = min(ip0 + 1, P-1)

        wh = ih - ih0; wd = idd - id0; wp = ip - ip0

        stride_p = 1; stride_d = P; stride_h = P * D

        def sample(block, ih_i, id_i, ip_i):
            idx = ip_i + id_i * stride_d + ih_i * stride_h
            return block[idx]

        def tri_interp(c000, c001, c010, c011, c100, c101, c110, c111):
            c00 = c000 * (1-wp) + c001 * wp
            c01 = c010 * (1-wp) + c011 * wp
            c10 = c100 * (1-wp) + c101 * wp
            c11 = c110 * (1-wp) + c111 * wp
            c0 = c00 * (1-wd) + c01 * wd
            c1 = c10 * (1-wd) + c11 * wd
            c = c0 * (1-wh) + c1 * wh
            return c

        r = tri_interp(
            sample(self.merl_r, ih0, id0, ip0), sample(self.merl_r, ih0, id0, ip1),
            sample(self.merl_r, ih0, id1, ip0), sample(self.merl_r, ih0, id1, ip1),
            sample(self.merl_r, ih1, id0, ip0), sample(self.merl_r, ih1, id0, ip1),
            sample(self.merl_r, ih1, id1, ip0), sample(self.merl_r, ih1, id1, ip1)
        ) * RED_SCALE

        g = tri_interp(
            sample(self.merl_g, ih0, id0, ip0), sample(self.merl_g, ih0, id0, ip1),
            sample(self.merl_g, ih0, id1, ip0), sample(self.merl_g, ih0, id1, ip1),
            sample(self.merl_g, ih1, id0, ip0), sample(self.merl_g, ih1, id0, ip1),
            sample(self.merl_g, ih1, id1, ip0), sample(self.merl_g, ih1, id1, ip1)
        ) * GREEN_SCALE

        b = tri_interp(
            sample(self.merl_b, ih0, id0, ip0), sample(self.merl_b, ih0, id0, ip1),
            sample(self.merl_b, ih0, id1, ip0), sample(self.merl_b, ih0, id1, ip1),
            sample(self.merl_b, ih1, id0, ip0), sample(self.merl_b, ih1, id0, ip1),
            sample(self.merl_b, ih1, id1, ip0), sample(self.merl_b, ih1, id1, ip1)
        ) * BLUE_SCALE

        return float(r), float(g), float(b)


def tone_map_and_gamma(rgb, exposure=1.0, gamma=2.2):
    # exposure: 线性乘子；simple filmic tone mapping: x/(x+1)
    r, g, b = rgb
    r = r * exposure; g = g * exposure; b = b * exposure
    r = r / (r + 1.0); g = g / (g + 1.0); b = b / (b + 1.0)
    # gamma 校正（输入在 [0,1]）
    invg = 1.0 / max(1e-6, gamma)
    r = max(0.0, min(1.0, r)) ** invg
    g = max(0.0, min(1.0, g)) ** invg
    b = max(0.0, min(1.0, b)) ** invg
    return r, g, b


class App:
    def __init__(self, root):
        root.title('MERL BRDF 插值查看器')
        self.brdf = BRDFData(); self.filepath = None

        # 基于屏幕大小计算目标窗口和控件尺寸（窗口占屏幕面积一半，宽高比 4:3）
        screen_w = root.winfo_screenwidth()
        screen_h = root.winfo_screenheight()
        target_h = int((0.375 * screen_w * screen_h) ** 0.5)
        target_w = int(target_h * 4 / 3)
        # 控件尺寸基于目标窗口尺寸
        entry_main_width = max(40, int(target_w // 15))
        rv_entry_width = max(8, int(target_w // 100))
        scale_length = max(200, int(target_w * 0.45))
        canvas_w = max(240, int(target_w * 0.28))
        canvas_h = max(120, int(target_h * 0.22))

        top = tk.Frame(root); top.pack(fill=tk.X, padx=6, pady=6)
        self.path_var = tk.StringVar()
        tk.Entry(top, textvariable=self.path_var, width=entry_main_width).pack(side=tk.LEFT, padx=4)
        tk.Button(top, text='打开 MERL binary', command=self.open_merl_binary).pack(side=tk.LEFT, padx=4)
        tk.Button(top, text='退出', command=root.quit).pack(side=tk.LEFT, padx=4)

        rv_frame = tk.Frame(root); rv_frame.pack(fill=tk.X, padx=6, pady=6)
        tk.Label(rv_frame, text='输入 rvectors (half, diff): hx hy hz  dx dy dz').pack(anchor='w')
        entry_frame = tk.Frame(rv_frame); entry_frame.pack(anchor='w')
        self.rv_vars = [tk.StringVar(value='0.0') for _ in range(6)]
        labels = ['hx','hy','hz','dx','dy','dz']
        for i, lab in enumerate(labels):
            tk.Label(entry_frame, text=lab).grid(row=0, column=i*2)
            tk.Entry(entry_frame, textvariable=self.rv_vars[i], width=rv_entry_width).grid(row=0, column=i*2+1)
        tk.Button(rv_frame, text='MERL 插值采样', command=self.sample_from_merl).pack(anchor='w', pady=4)

        # 曝光与 gamma 控件
        ctrl_frame = tk.Frame(root); ctrl_frame.pack(fill=tk.X, padx=6, pady=6)
        tk.Label(ctrl_frame, text='曝光:').grid(row=0, column=0)
        self.exposure_var = tk.DoubleVar(value=1.0)
        tk.Scale(ctrl_frame, from_=0.01, to=10.0, orient=tk.HORIZONTAL, resolution=0.01, variable=self.exposure_var, length=scale_length).grid(row=0, column=1, sticky='we')
        tk.Label(ctrl_frame, text='gamma:').grid(row=1, column=0)
        self.gamma_var = tk.DoubleVar(value=2.2)
        tk.Scale(ctrl_frame, from_=0.1, to=4.0, orient=tk.HORIZONTAL, resolution=0.01, variable=self.gamma_var, length=scale_length).grid(row=1, column=1, sticky='we')

        self.rgb_text = tk.StringVar(); tk.Label(root, textvariable=self.rgb_text, font=('Courier', 12)).pack(anchor='w', padx=6)
        preview_frame = tk.Frame(root); preview_frame.pack(anchor='w', pady=6)
        tk.Label(preview_frame, text='颜色预览:').pack(side=tk.LEFT)
        self.canvas = tk.Canvas(preview_frame, width=canvas_w, height=canvas_h, bg='white', bd=1, relief=tk.SUNKEN)
        self.canvas.pack(side=tk.LEFT, padx=8)
        self.color_rect = self.canvas.create_rectangle(2,2,canvas_w-2,canvas_h-2, fill='#000000')

    def open_merl_binary(self):
        path = filedialog.askopenfilename(initialdir='.', title='打开 MERL binary 文件', filetypes=[('BRDF binary','*.binary'),('All files','*.*')])
        if not path: return
        try:
            self.brdf.load_binary_merl(path)
        except Exception as e:
            messagebox.showerror('加载 MERL 二进制失败', str(e)); return
        messagebox.showinfo('加载完成', f'已加载 MERL 文件: {os.path.basename(path)}')

    def sample_from_merl(self):
        if not getattr(self.brdf, 'loaded_merl', False):
            messagebox.showwarning('未加载 MERL', '请先加载 .binary 文件')
            return
        try:
            vals = [float(v.get()) for v in self.rv_vars]
        except Exception:
            messagebox.showerror('输入错误', '请在 rvectors 输入框中填写 6 个浮点数'); return
        hx, hy, hz, dx, dy, dz = vals
        # 归一化（若需要），并保证 z >= 0
        hn = math.sqrt(hx*hx + hy*hy + hz*hz); dn = math.sqrt(dx*dx + dy*dy + dz*dz)
        normalized = False
        if hn > 0 and abs(hn - 1.0) > 1e-6:
            hx /= hn; hy /= hn; hz /= hn; normalized = True
        if dn > 0 and abs(dn - 1.0) > 1e-6:
            dx /= dn; dy /= dn; dz /= dn; normalized = True
        if normalized:
            messagebox.showinfo('提示', '输入向量已自动归一化。')
        if hz < 0.0: hz = 0.0
        if dz < 0.0: dz = 0.0

        hz_c = max(0.0, min(1.0, hz)); dz_c = max(0.0, min(1.0, dz))
        theta_half = math.acos(hz_c); theta_diff = math.acos(dz_c); phi_diff = math.atan2(dy, dx)

        r, g, b = self.brdf.merl_trilinear_sample(theta_half, theta_diff, phi_diff)
        self.rgb_text.set(f'R={r:.6f}  G={g:.6f}  B={b:.6f}  (MERL 插值结果)')

        # exposure + tone-map + gamma
        exposure = float(self.exposure_var.get()); gamma = float(self.gamma_var.get())
        r_t, g_t, b_t = tone_map_and_gamma((r, g, b), exposure=exposure, gamma=gamma)
        rgb_u8 = tuple(int(max(0, min(1, c)) * 255) for c in (r_t, g_t, b_t))
        color = '#%02x%02x%02x' % rgb_u8
        self.canvas.itemconfig(self.color_rect, fill=color)


if __name__ == '__main__':
    root = tk.Tk()
    root.geometry("1000x700")    # 增大初始窗口 (宽 x 高)
    root.minsize(600,400)        # 可选：设置最小尺寸
    app = App(root)
    root.mainloop()
