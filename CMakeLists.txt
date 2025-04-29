# 设置 CMake 最低版本
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(QuantumSimulationProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 库
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 包含 Qt 的 MOC（元对象编译器）功能
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# 定义非 Qt 源文件列表（不包含 main 目录下有 main 函数的文件）
file(GLOB_RECURSE NON_QT_SOURCES
    src/algorithm/*.cpp
    src/circuit/*.cpp
    src/common/*.cpp
    src/parser/*.cpp
    src/state/*.cpp
    src/uni_mat/*.cpp
)

# 添加非 Qt 库
add_library(non_qt_lib STATIC ${NON_QT_SOURCES})

# 遍历 main 目录下的 .cpp 文件，为每个文件创建可执行文件
file(GLOB MAIN_SOURCES main/*.cpp)
foreach(MAIN_SOURCE ${MAIN_SOURCES})
    get_filename_component(EXECUTABLE_NAME ${MAIN_SOURCE} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${MAIN_SOURCE})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE non_qt_lib)
endforeach()

# 定义 Qt 源文件列表
file(GLOB_RECURSE QT_SOURCES
    ui/*.cpp
    ui/*.h
)

# 添加可执行文件（Qt 应用）
add_executable(quiver ${QT_SOURCES})

# 链接非 Qt 库到 Qt 应用
target_link_libraries(quiver PRIVATE non_qt_lib)

# 链接 Qt 库到 Qt 应用
target_link_libraries(quiver PRIVATE Qt5::Widgets)

# 设置头文件搜索路径
target_include_directories(non_qt_lib PUBLIC
    main
    src
    # src/algorithm
    # src/circuit
    # src/common
    # src/parser
    # src/state
    # src/uni_mat
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(quiver PUBLIC
    main
    src
    ui
    .
    ${CMAKE_CURRENT_BINARY_DIR}
)

# target_compile_definitions(quantum_app PRIVATE QT_NO_KEYWORDS)