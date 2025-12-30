import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as animation

def plot_intensity(theta, intensity):
    plt.figure(figsize=(8,4))
    plt.plot(np.degrees(theta), intensity)
    plt.xlabel("衍射角 θ (度)")
    plt.ylabel("归一化光强")
    plt.title("双缝干涉光强分布")
    plt.grid(True)
    plt.show()

def plot_3d_intensity(screen_x, screen_y, intensity_2d):
    fig = plt.figure(figsize=(8,6))
    ax = fig.add_subplot(111, projection='3d')
    ax.plot_surface(screen_x, screen_y, intensity_2d, cmap='viridis')
    ax.set_xlabel('屏幕X坐标 (m)')
    ax.set_ylabel('屏幕Y坐标 (m)')
    ax.set_zlabel('光强')
    ax.set_title('二维屏幕上的光强分布')
    plt.show()

def animate_intensity(theta, wavelength, slit_width, slit_distance, num_slits):
    fig, ax = plt.subplots()
    line, = ax.plot([], [], lw=2)
    ax.set_xlim(np.degrees(theta[0]), np.degrees(theta[-1]))
    ax.set_ylim(0, 1)
    ax.set_xlabel("衍射角 θ (度)")
    ax.set_ylabel("归一化光强")
    ax.set_title("双缝干涉动态演示")

    def init():
        line.set_data([], [])
        return (line,)

    def update(frame):
        # 例如动态改变缝距frame参数
        d = slit_distance * (0.5 + 0.5 * np.sin(frame / 10))
        from simulation import total_intensity
        intensity = total_intensity(theta, wavelength, slit_width, d, num_slits)
        line.set_data(np.degrees(theta), intensity)
        return (line,)

    ani = animation.FuncAnimation(fig, update, frames=np.arange(0, 100), init_func=init, blit=True, interval=100)
    plt.show()

def plot_intensity_grayscale(intensity_1d):
    """
    生成一维明暗条纹的灰度图像显示
    intensity_1d: 一维光强数组，归一化范围0~1
    """
    # 将一维光强扩展成二维条纹（高度任意，比如100像素）
    stripes = np.tile(intensity_1d, (100, 1))

    plt.figure(figsize=(10, 2))
    plt.imshow(stripes, cmap='gray', aspect='auto', vmin=0, vmax=1)
    plt.title("双缝干涉明暗条纹 - 灰度图模拟")
    plt.axis('off')
    plt.show()

def plot_intensity_color(intensity_1d):
    """
    生成一维明暗条纹的彩色图像显示，使用伪彩色映射
    intensity_1d: 一维光强数组，归一化范围0~1
    """
    stripes = np.tile(intensity_1d, (100, 1))

    plt.figure(figsize=(10, 2))
    plt.imshow(stripes, cmap='inferno', aspect='auto', vmin=0, vmax=1)
    plt.title("双缝干涉明暗条纹 - 彩色图模拟")
    plt.axis('off')
    plt.show()