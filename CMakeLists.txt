cmake_minimum_required(VERSION 3.20)
project(DiabloEdit VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 查找
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 源文件
set(CORE_SOURCES
    src/core/BinDataStream.cpp
    src/core/MetaData.cpp
    src/core/D2Item.cpp
    src/core/D2S_Struct.cpp
    src/core/ImageManager.cpp
    src/core/quicklz.cpp
)

set(CORE_HEADERS
    src/core/D2Types.h
    src/core/D2Version.h
    src/core/BinDataStream.h
    src/core/MayExist.h
    src/core/MetaData.h
    src/core/DataManagerFwd.h
    src/core/D2Item.h
    src/core/D2S_Struct.h
    src/core/ImageManager.h
    src/core/quicklz.h
    src/core/quicklz.c.h
    src/core/compress_quicklz.h
)

set(DATA_SOURCES
    src/data/DataManager.cpp
)

set(DATA_HEADERS
    src/data/DataManager.h
)

set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/CharInfoWidget.cpp
    src/ui/WaypointsWidget.cpp
    src/ui/QuestInfoWidget.cpp
    src/ui/SkillsDialog.cpp
    src/ui/SelectCharDialog.cpp
    src/ui/ItemTooltip.cpp
    src/ui/ItemGridWidget.cpp
    src/ui/NewItemDialog.cpp
    src/ui/FoundryDialog.cpp
)

set(UI_HEADERS
    src/ui/MainWindow.h
    src/ui/CharInfoWidget.h
    src/ui/WaypointsWidget.h
    src/ui/QuestInfoWidget.h
    src/ui/SkillsDialog.h
    src/ui/SelectCharDialog.h
    src/ui/ItemTooltip.h
    src/ui/ItemGridWidget.h
    src/ui/NewItemDialog.h
    src/ui/FoundryDialog.h
)

# 主可执行文件
add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${UI_SOURCES}
    src/main.cpp
    resources/resources.qrc
)

# MinGW: 去掉 Windows 命令行窗口
if(MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# 数据文件复制到构建目录
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/data/
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data
)

# 数据生成工具
option(BUILD_DATA_TOOLS "Build data generation tools" ON)
if(BUILD_DATA_TOOLS)
    add_subdirectory(tools/generate_data)
    add_subdirectory(tools/verify_property)
endif()
