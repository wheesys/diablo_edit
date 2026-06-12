# generate_data - 独立命令行工具，将 .txt 编译为 QuickLZ 压缩的 .dat
# 不依赖 Qt，纯 C++17 + QuickLZ

cmake_minimum_required(VERSION 3.16)
project(generate_data LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QuickLZ 源码路径（相对于项目根目录）
set(QUICKLZ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/core")

# 源文件
add_executable(generate_data
    main.cpp
    ${QUICKLZ_DIR}/quicklz.cpp
)

# 头文件包含路径
target_include_directories(generate_data PRIVATE
    ${QUICKLZ_DIR}
)

# Windows 下去掉命令行窗口（MinGW）
if(MINGW)
    set_target_properties(generate_data PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()
