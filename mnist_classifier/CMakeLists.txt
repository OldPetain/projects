# CMakeLists.txt for MNIST Classifier Project
cmake_minimum_required(VERSION 3.10)
project(MNISTClassifier)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 LibTorch 根目录
set(TORCH_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/libtorch") # 假设 libtorch 在项目根目录

# 查找 LibTorch
find_package(Torch REQUIRED PATHS ${TORCH_INSTALL_PREFIX})
if (NOT Torch_FOUND)
    message(FATAL_ERROR "Could not find LibTorch. Please set TORCH_INSTALL_PREFIX correctly or ensure LibTorch is in the standard search paths.")
endif()

message(STATUS "Found LibTorch: ${TORCH_LIBRARIES}")

include_directories(include)

file(GLOB SOURCES
    src/*.cpp
)

add_executable(test_mnist
    ${SOURCES}
    test/main.cpp
)

target_link_libraries(test_mnist PRIVATE ${TORCH_LIBRARIES})

set_property(TARGET test_mnist PROPERTY CXX_STANDARD 17)

# 对于 CUDA 支持，如果需要
# if (CUDA_FOUND)
#     target_link_libraries(mnist_cnn PRIVATE ${CUDA_LIBRARIES})
# endif()