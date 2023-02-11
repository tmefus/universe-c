cmake_minimum_required(VERSION 3.24)
project(Civilization C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..) # 可执行文件位置（与动态库同文件夹）
add_executable(Civilization main.c)
target_link_libraries(Civilization libfreeglut.a opengl32.dll libglu32.a)