#!/usr/bin/env python3
import os

def fix_venv(venv_path):
    activate_path = os.path.join(venv_path, "bin", "activate")
    cfg_path = os.path.join(venv_path, "pyvenv.cfg")

    if not os.path.exists(activate_path) or not os.path.exists(cfg_path):
        return False

    # 新的绝对路径
    new_path = os.path.abspath(venv_path)

    # 修复 activate 文件
    with open(activate_path, "r") as f:
        content = f.read()
    if "VIRTUAL_ENV=" in content:
        import re
        content = re.sub(r'VIRTUAL_ENV=.*', f'VIRTUAL_ENV="{new_path}"', content)
        with open(activate_path, "w") as f:
            f.write(content)

    # 修复 pyvenv.cfg 文件
    lines = []
    with open(cfg_path, "r") as f:
        for line in f:
            if line.startswith("home ="):
                lines.append(line)  # 不改 Python 解释器路径
            elif line.startswith("include-system-site-packages"):
                lines.append(line)
            elif line.startswith("version"):
                lines.append(line)
            elif line.startswith("virtualenv"):
                lines.append(line)
            elif line.startswith(""):
                lines.append(line)
            else:
                lines.append(line)

    # 添加 virtual environment 的新路径（部分 Python 版本不需要）
    if not any("virtualenv =" in line for line in lines):
        lines.append(f"virtualenv = {new_path}\n")

    with open(cfg_path, "w") as f:
        f.writelines(lines)

    print(f"✅ Fixed venv at: {venv_path}")
    return True


def main():
    for root, dirs, files in os.walk("."):
        if "bin" in dirs and "lib" in dirs and "pyvenv.cfg" in files:
            venv_path = os.path.abspath(root)
            fix_venv(venv_path)


if __name__ == "__main__":
    main()
