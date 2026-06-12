# verify_property - 验证 property.dat 能否正确解析 3.1 存档
# 依赖 Qt，因为需复用 DataManager 和 D2S_Struct

cmake_minimum_required(VERSION 3.16)
project(verify_property LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

add_executable(verify_property
    main.cpp
    ${SRC_DIR}/core/BinDataStream.cpp
    ${SRC_DIR}/core/MetaData.cpp
    ${SRC_DIR}/core/D2Item.cpp
    ${SRC_DIR}/core/D2S_Struct.cpp
    ${SRC_DIR}/core/quicklz.cpp
    ${SRC_DIR}/data/DataManager.cpp
)

target_include_directories(verify_property PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/core
    ${SRC_DIR}/data
    ${SRC_DIR}/ui
)

target_link_libraries(verify_property PRIVATE Qt6::Core)
