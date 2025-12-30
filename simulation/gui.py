import matplotlib
matplotlib.rcParams['font.family'] = 'WenQuanYi Zen Hei'  # 强制使用中文字体
matplotlib.rcParams['axes.unicode_minus'] = False  # 负号显示正常
import tkinter as tk
from tkinter import ttk, messagebox
import numpy as np
from simulation import total_intensity
import plot_utils

class DoubleSlitApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("多缝干涉与单缝衍射仿真")
        self.geometry("700x500")
        self.create_widgets()
        self.setup_defaults()

    def create_widgets(self):
        frm = ttk.Frame(self)
        frm.pack(padx=10, pady=10, fill=tk.X)

        # 参数输入框
        labels = ["波长 (nm)", "缝间距 (μm)", "缝宽 (μm)", "屏幕距离 (m)", "通光缝数 (个)"]
        self.entries = {}
        for i, label in enumerate(labels):
            ttk.Label(frm, text=label).grid(row=i, column=0, sticky=tk.W, pady=3)
            ent = ttk.Entry(frm, width=20)
            ent.grid(row=i, column=1)
            self.entries[label] = ent

        # 推荐值显示
        self.recommend_label = ttk.Label(frm, text="推荐范围:\n波长: 400-700 nm\n缝间距: 10-500 μm\n缝宽: 1-50 μm\n屏幕距离: 0.5-2 m\n通光缝数: 2-10")
        self.recommend_label.grid(row=0, column=2, rowspan=5, padx=20)

        # 按钮
        btn_frame = ttk.Frame(self)
        btn_frame.pack(pady=10)

        self.calc_btn = ttk.Button(btn_frame, text="计算光强分布", command=self.on_calculate)
        self.calc_btn.grid(row=0, column=0, padx=10)

        self.anim_btn = ttk.Button(btn_frame, text="动画演示", command=self.on_animate)
        self.anim_btn.grid(row=0, column=1, padx=10)
        
        self.gray_btn = ttk.Button(btn_frame, text="灰度条纹模拟", command=self.on_plot_gray)
        self.gray_btn.grid(row=1, column=0, padx=10, pady=5)

        self.color_btn = ttk.Button(btn_frame, text="彩色条纹模拟", command=self.on_plot_color)
        self.color_btn.grid(row=1, column=1, padx=10, pady=5)

    def setup_defaults(self):
        self.entries["波长 (nm)"].insert(0, "632.8")
        self.entries["缝间距 (μm)"].insert(0, "250")
        self.entries["缝宽 (μm)"].insert(0, "50")
        self.entries["屏幕距离 (m)"].insert(0, "1")
        self.entries["通光缝数 (个)"].insert(0, "2")

    def on_calculate(self):
        try:
            wavelength = float(self.entries["波长 (nm)"].get()) * 1e-9
            slit_distance = float(self.entries["缝间距 (μm)"].get()) * 1e-6
            slit_width = float(self.entries["缝宽 (μm)"].get()) * 1e-6
            screen_distance = float(self.entries["屏幕距离 (m)"].get())
            num_slits = int(self.entries["通光缝数 (个)"].get())

            if wavelength <= 0 or slit_distance <= 0 or slit_width <= 0 or screen_distance <= 0 or num_slits <= 0:
                raise ValueError

            # 计算角度范围
            theta = np.linspace(-0.01, 0.01, 1000)  # 角度范围较小，弧度单位
            intensity = total_intensity(theta, wavelength, slit_width, slit_distance, num_slits)

            plot_utils.plot_intensity(theta, intensity)

        except ValueError:
            messagebox.showerror("输入错误", "请输入有效的正数参数")

    def on_animate(self):
        try:
            wavelength = float(self.entries["波长 (nm)"].get()) * 1e-9
            slit_distance = float(self.entries["缝间距 (μm)"].get()) * 1e-6
            slit_width = float(self.entries["缝宽 (μm)"].get()) * 1e-6
            num_slits = int(self.entries["通光缝数 (个)"].get())
            theta = np.linspace(-0.01, 0.01, 500)

            plot_utils.animate_intensity(theta, wavelength, slit_width, slit_distance, num_slits)

        except ValueError:
            messagebox.showerror("输入错误", "请输入有效的正数参数")
    def on_plot_gray(self):
        try:
            wavelength = float(self.entries["波长 (nm)"].get()) * 1e-9
            slit_distance = float(self.entries["缝间距 (μm)"].get()) * 1e-6
            slit_width = float(self.entries["缝宽 (μm)"].get()) * 1e-6
            num_slits = int(self.entries["通光缝数 (个)"].get())

            theta = np.linspace(-0.01, 0.01, 1000)
            intensity = total_intensity(theta, wavelength, slit_width, slit_distance, num_slits)

            from plot_utils import plot_intensity_grayscale
            plot_intensity_grayscale(intensity)

        except ValueError:
            messagebox.showerror("输入错误", "请输入有效的正数参数")

    def on_plot_color(self):
        try:
            wavelength = float(self.entries["波长 (nm)"].get()) * 1e-9
            slit_distance = float(self.entries["缝间距 (μm)"].get()) * 1e-6
            slit_width = float(self.entries["缝宽 (μm)"].get()) * 1e-6
            num_slits = int(self.entries["通光缝数 (个)"].get())

            theta = np.linspace(-0.01, 0.01, 1000)
            intensity = total_intensity(theta, wavelength, slit_width, slit_distance, num_slits)

            from plot_utils import plot_intensity_color
            plot_intensity_color(intensity)

        except ValueError:
            messagebox.showerror("输入错误", "请输入有效的正数参数")
