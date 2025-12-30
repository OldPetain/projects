编译：
cd ~/SDL/project/build
cmake..
make

执行：
cd ..  # 返回到 ~/SDL/project 目录
./bin/game

如果你想在 编译前 清理之前编译生成的文件（例如，你修改了项目配置或依赖），可以在 build 目录下运行 make clean 命令。

# 注意：
1. 请确保 SDL 库已经正确安装，并且在编译前已经设置好环境变量。
2. 请确保项目目录下有 CMakeLists.txt 文件，并且已经正确配置。
