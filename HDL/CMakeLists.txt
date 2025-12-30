cmake_minimum_required(VERSION 3.10)
project(game)

find_package(SDL2 REQUIRED)
find_library(SDL_IMAGE SDL2_image REQUIRED)
find_library(SDL_TTF SDL2_ttf REQUIRED)
aux_source_directory(./src/ SRC_LIST)
include_directories(./include ${SDL2_INCLUDE_DIR})
add_executable(game ${SRC_LIST})

target_link_libraries(game ${SDL2_LIBRARIES})
target_link_libraries(game ${SDL_IMAGE} ${SDL_TTF})

set(EXECUTABLE_OUTPUT_PATH ../bin)