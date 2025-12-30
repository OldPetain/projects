# 此代码自动检测虚拟环境中关键包是否能导入，并自动重装缺失或绑定错误的包。（修复路径及安装缺失包）
#!/usr/bin/env python3
import os
import sys
import subprocess

# 要检测和修复的关键包
REQUIRED_PACKAGES = ["numpy", "torch"]

def fix_shebang_and_cfg(venv_path):
    """修复 bin 下 shebang 和 pyvenv.cfg home 路径"""
    bin_path = os.path.join(venv_path, "bin")
    pyvenv_cfg = os.path.join(venv_path, "pyvenv.cfg")
    python_exe = os.path.join(bin_path, "python")

    fixed = False

    # 修复 shebang
    for fname in os.listdir(bin_path):
        fpath = os.path.join(bin_path, fname)
        if not os.path.isfile(fpath):
            continue
        try:
            with open(fpath, "rb") as f:
                lines = f.read().splitlines()
            if lines and lines[0].startswith(b"#!") and b"python" in lines[0]:
                old = lines[0].decode()
                new = f"#!{python_exe}"
                if old != new:
                    lines[0] = new.encode()
                    with open(fpath, "wb") as f:
                        f.write(b"\n".join(lines) + b"\n")
                    print(f"🔧 修复 shebang: {fpath}\n       {old} -> {new}")
                    fixed = True
        except Exception as e:
            print(f"⚠️ 跳过 {fpath}: {e}")

    # 修复 pyvenv.cfg
    try:
        with open(pyvenv_cfg, "r") as f:
            lines = f.readlines()
        new_lines = []
        updated = False
        for line in lines:
            if line.startswith("home ="):
                old = line.strip()
                line = f"home = {venv_path}\n"
                print(f"🔧 修复 pyvenv.cfg: {old} -> {line.strip()}")
                updated = True
            new_lines.append(line)
        if updated:
            with open(pyvenv_cfg, "w") as f:
                f.writelines(new_lines)
            fixed = True
    except Exception as e:
        print(f"⚠️ 无法修改 pyvenv.cfg: {e}")

    return fixed

def check_and_reinstall_packages(venv_path):
    """检测关键包是否可用，不可用则自动重装"""
    python_bin = os.path.join(venv_path, "bin", "python")
    pip_bin = os.path.join(venv_path, "bin", "pip")
    for pkg in REQUIRED_PACKAGES:
        try:
            subprocess.run(
                [python_bin, "-c", f"import {pkg}"],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            print(f"   ✅ {pkg} 已安装且可用")
        except subprocess.CalledProcessError:
            print(f"   ❌ {pkg} 不可用，尝试重新安装...")
            try:
                subprocess.run([pip_bin, "install", "--force-reinstall", pkg], check=True)
                print(f"       ✅ {pkg} 安装完成")
            except subprocess.CalledProcessError:
                print(f"       ⚠️ {pkg} 安装失败，请手动处理")

def fix_and_check_venv(venv_path):
    print(f"\n🔎 检查虚拟环境: {venv_path}")
    fix_shebang_and_cfg(venv_path)
    check_and_reinstall_packages(venv_path)

def scan_all_venvs(root_dir="."):
    """扫描当前目录及子目录下的虚拟环境"""
    for root, dirs, files in os.walk(root_dir):
        if "bin" in dirs and "pyvenv.cfg" in files:
            venv_path = os.path.abspath(root)
            fix_and_check_venv(venv_path)

if __name__ == "__main__":
    scan_all_venvs()
