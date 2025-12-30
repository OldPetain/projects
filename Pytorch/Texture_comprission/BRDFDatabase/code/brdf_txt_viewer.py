#!/usr/bin/env python3
"""
BRDF 文本查表查看器（仅 .txt）

用法:
    python3 brdf_txt_viewer.py

说明:
    只打开由 `BRDFRead.cpp` 生成的采样文本文件（例如 `aluminium.txt`），
    使用滑块或 +/- 按钮选择 i,j,k,l 索引，显示对应角度与 R/G/B 值，并在右侧显示颜色预览。

依赖: Python 标准库 (tkinter)
"""

import tkinter as tk
from tkinter import filedialog, messagebox
import math

# 与 BRDFRead.cpp 中使用的采样维度相同
DIM_I = 16
DIM_J = 64
DIM_K = 16
DIM_L = 64
TOTAL_LINES = DIM_I * DIM_J * DIM_K * DIM_L


class BRDFData:
    def __init__(self):
        self.loaded = False
        self.data = None

    def load_txt(self, path):
        with open(path, 'r') as f:
            lines = [ln.strip() for ln in f if ln.strip()]
        n = len(lines)
        if n != TOTAL_LINES:
            raise ValueError(f"Unexpected line count: {n} (expected {TOTAL_LINES}).")
        arr = [None] * TOTAL_LINES
        for idx, ln in enumerate(lines):
            parts = ln.split()
            if len(parts) < 3:
                raise ValueError(f"Line {idx+1} does not contain 3 floats: {ln}")
            r = float(parts[0]); g = float(parts[1]); b = float(parts[2])
            arr[idx] = (r, g, b)
        self.data = arr
        self.loaded = True

    def get_index(self, i, j, k, l):
        return ((i * DIM_J + j) * DIM_K + k) * DIM_L + l

    def lookup(self, i, j, k, l):
        if not self.loaded:
            return None
        return self.data[self.get_index(i, j, k, l)]


def index_to_angles(i, j, k, l):
    n = DIM_I
    theta_in = i * 0.5 * math.pi / n
    phi_in = j * 2.0 * math.pi / (4*n)
    theta_out = k * 0.5 * math.pi / n
    phi_out = l * 2.0 * math.pi / (4*n)
    return theta_in, phi_in, theta_out, phi_out


class App:
    def __init__(self, root):
        self.root = root
        root.title('BRDF 文本查表查看器')
        # 计算初始窗口尺寸：按屏幕大小取面积的一半，保持 16:9 比例
        sw = root.winfo_screenwidth()
        sh = root.winfo_screenheight()
        screen_area = sw * sh
        half_area = screen_area * 0.5
        aspect = 16.0 / 9.0
        # 从面积和比例求宽高：w = sqrt(area * aspect), h = w / aspect
        win_w = int(max(300, math.sqrt(half_area * aspect)))
        win_h = int(max(200, win_w / aspect))
        root.geometry(f"{win_w}x{win_h}")
        # 最小尺寸避免过小
        root.minsize(640, 360)
        self.brdf = BRDFData()
        self.path_var = tk.StringVar()

        top = tk.Frame(root)
        top.pack(fill=tk.X, padx=6, pady=6)
        # 路径输入与按钮：使用 fill/expand 以响应窗口大小变化
        tk.Entry(top, textvariable=self.path_var).pack(side=tk.LEFT, padx=4, fill=tk.X, expand=True)
        tk.Button(top, text='打开 txt 文件', command=self.open_file).pack(side=tk.LEFT, padx=4, fill=tk.BOTH, expand=True)
        tk.Button(top, text='退出', command=root.quit).pack(side=tk.LEFT, padx=4, fill=tk.BOTH, expand=True)

        mid = tk.Frame(root)
        # mid 可横向与纵向伸缩，内部索引列会拉伸
        mid.pack(fill=tk.BOTH, padx=6, pady=6, expand=True)

        self.vars = {}
        self.scales = {}
        params = [
            ('i (theta_in)', 0, DIM_I-1),
            ('j (phi_in)', 0, DIM_J-1),
            ('k (theta_out)', 0, DIM_K-1),
            ('l (phi_out)', 0, DIM_L-1),
        ]

        for idx, (name, lo, hi) in enumerate(params):
            frame = tk.Frame(mid, relief=tk.GROOVE, bd=1)
            # 每一列在 mid 中横纵都可伸缩
            frame.grid(row=0, column=idx, padx=4, pady=4, sticky='nsew')
            tk.Label(frame, text=name).pack(fill=tk.X)
            v = tk.IntVar(); v.set(lo); self.vars[name] = v
            # 让按钮在父容器中可伸缩
            tk.Button(frame, text='+', command=lambda n=name: self.step(n, +1)).pack(fill=tk.BOTH, expand=True)
            tk.Button(frame, text='-', command=lambda n=name: self.step(n, -1)).pack(fill=tk.BOTH, expand=True)
            # 让 Scale 随容器伸缩
            scl = tk.Scale(frame, from_=lo, to=hi, orient=tk.HORIZONTAL, variable=v, command=lambda ev, n=name: self.on_change(n))
            scl.pack(fill=tk.BOTH, expand=True)
            self.scales[name] = scl

        # 让 mid 的每一列随窗口宽度伸缩
        for c in range(len(params)):
            mid.grid_columnconfigure(c, weight=1)
        # 允许索引行在垂直方向也伸缩
        mid.grid_rowconfigure(0, weight=1)

        self.angle_text = tk.StringVar()
        tk.Label(root, textvariable=self.angle_text, justify=tk.LEFT).pack(anchor='w', padx=6)

        self.rgb_text = tk.StringVar()
        tk.Label(root, textvariable=self.rgb_text, font=('Courier', 12)).pack(anchor='w')

        preview_frame = tk.Frame(root)
        # 预览区域可伸缩，占剩余空间
        preview_frame.pack(fill=tk.BOTH, padx=6, pady=6, expand=True)
        tk.Label(preview_frame, text='颜色预览:').pack(side=tk.LEFT)
        # 颜色预览画布：初始大小按窗口比例设置，并随窗口伸缩
        # 初始画布大小：窗口宽度的 30% 高度的 40%（近似）
        init_cw = max(120, int(win_w * 0.3))
        init_ch = max(60, int(win_h * 0.4))
        self.canvas = tk.Canvas(preview_frame, width=init_cw, height=init_ch, bg='white', bd=1, relief=tk.SUNKEN)
        self.canvas.pack(side=tk.LEFT, padx=8, fill=tk.BOTH, expand=True)
        self.color_rect = self.canvas.create_rectangle(2,2,init_cw-2,init_ch-2, fill='#000000')
        # 当画布大小变化时，调整矩形和图片位置
        self.canvas.bind('<Configure>', self._on_canvas_resize)

        tk.Label(root, text='使用滑块或 +/- 按钮选择索引。角度以弧度显示。').pack(anchor='w', padx=6)

        self.update_display()

    def open_file(self):
        path = filedialog.askopenfilename(initialdir='.', title='打开 BRDF 文本文件', filetypes=[('Text files','*.txt'),('All files','*.*')])
        if not path: return
        self.path_var.set(path)
        try:
            self.brdf.load_txt(path)
        except Exception as e:
            messagebox.showerror('加载失败', str(e)); return
        self.vars['i (theta_in)'].set(0); self.vars['j (phi_in)'].set(0)
        self.vars['k (theta_out)'].set(0); self.vars['l (phi_out)'].set(0)
        self.update_display()

    def _on_canvas_resize(self, event):
        # 更新矩形到新的画布边界
        w = max(4, event.width - 2)
        h = max(4, event.height - 2)
        try:
            self.canvas.coords(self.color_rect, 2, 2, w, h)
        except Exception:
            pass
        # 如果存在渲染图片，移动到左上角
        if getattr(self, '_img_on_canvas', None) is not None:
            try:
                self.canvas.coords(self._img_on_canvas, 2, 2)
            except Exception:
                pass

    def step(self, name, delta):
        v = self.vars[name]; cur = v.get(); lo = int(self.scales[name]['from']); hi = int(self.scales[name]['to'])
        nxt = max(lo, min(hi, cur + delta)); v.set(nxt); self.update_display()

    def on_change(self, name):
        self.update_display()

    def update_display(self):
        i = self.vars['i (theta_in)'].get(); j = self.vars['j (phi_in)'].get(); k = self.vars['k (theta_out)'].get(); l = self.vars['l (phi_out)'].get()
        theta_in, phi_in, theta_out, phi_out = index_to_angles(i,j,k,l)
        self.angle_text.set(f'theta_in={theta_in:.4f}, phi_in={phi_in:.4f}, theta_out={theta_out:.4f}, phi_out={phi_out:.4f}')
        rgb = self.brdf.lookup(i,j,k,l) if self.brdf.loaded else None
        if rgb is None:
            self.rgb_text.set('尚未加载数据。'); color = '#000000'
        else:
            r,g,b = rgb
            self.rgb_text.set(f'R={r:.6f}  G={g:.6f}  B={b:.6f}  (仅用于预览)')
            clamped = [max(0.0, r), max(0.0, g), max(0.0, b)]; maxc = max(clamped)
            if maxc > 1.0: clamped = [c / maxc for c in clamped]
            rgb_u8 = tuple(int(max(0, min(1, c)) * 255) for c in clamped)
            color = '#%02x%02x%02x' % rgb_u8
        self.canvas.itemconfig(self.color_rect, fill=color)


if __name__ == '__main__':
    root = tk.Tk(); app = App(root); root.mainloop()
