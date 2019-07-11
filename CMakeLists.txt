cmake_minimum_required(VERSION 3.10)
project(Novel)

set(CMAKE_CXX_STANDARD 11)
# 添加需要链接的lib文件路径
#link_libraries("/usr/local/lib/libgumbo.a")
link_libraries("/usr/local/lib/libgumbo.so")

add_executable(Novel
        src/main.cpp
        src/config.h
        src/test/Test.cpp
        src/test/Test.h
        src/checkhtml/Novel.cpp
        src/checkhtml/Novel.h)
