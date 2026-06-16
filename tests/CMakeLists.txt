# Test CMakeLists — diablo_edit test suite
# Required: BUILD_TESTS=ON, Qt6::Test available

# ====== Test utilities ======
add_library(test_utils STATIC
    test_utils.cpp
)
target_include_directories(test_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/data
)
target_link_libraries(test_utils PUBLIC Qt6::Core Qt6::Gui Qt6::Test)

target_compile_definitions(test_utils PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/.."
)

# ====== test_imagemanager ======
add_executable(test_imagemanager
    core/test_imagemanager.cpp
    $<TARGET_OBJECTS:diablo_core_obj>
    $<TARGET_OBJECTS:diablo_data_obj>
)
target_include_directories(test_imagemanager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/data
)
target_link_libraries(test_imagemanager PRIVATE
    Qt6::Core Qt6::Gui Qt6::Test test_utils
)
add_test(NAME ImageManager COMMAND test_imagemanager)

# ====== test_d2item ======
add_executable(test_d2item
    core/test_d2item.cpp
    $<TARGET_OBJECTS:diablo_core_obj>
    $<TARGET_OBJECTS:diablo_data_obj>
)
target_include_directories(test_d2item PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/data
)
target_link_libraries(test_d2item PRIVATE
    Qt6::Core Qt6::Gui Qt6::Test test_utils
)
add_test(NAME D2Item COMMAND test_d2item)

# ====== test_itemgridwidget (visual) ======
# Needs: ItemGridWidget.cpp (+ its own moc), g_dataMgr, ImageManager
# ItemGridWidget.cpp references FoundryDialog and NewItemDialog, so their
# implementations must be compiled into the test executable too.
add_executable(test_itemgridwidget
    ui/test_itemgridwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ItemGridWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/FoundryDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/NewItemDialog.cpp
    $<TARGET_OBJECTS:diablo_core_obj>
    $<TARGET_OBJECTS:diablo_data_obj>
)
target_include_directories(test_itemgridwidget PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/data
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui
)
target_link_libraries(test_itemgridwidget PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test test_utils
)
add_test(NAME ItemGridWidget COMMAND test_itemgridwidget)
