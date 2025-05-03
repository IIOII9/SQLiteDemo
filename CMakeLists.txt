cmake_minimum_required(VERSION 3.16)
# 设置默认构建类型为 RelWithDebInfo（如果没有通过命令行传入）
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

project(SQLiteDemo VERSION 1.0 LANGUAGES CXX)
# 告诉 CMake 目标程序要使用 C++17 标准。
set(CMAKE_CXX_STANDARD 17)
# 要求编译器必须支持并使用指定标准（如 C++17），否则配置会失败。
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 禁止使用编译器特定的扩展（如 GNU 的 -std=gnu++17），改用标准的 -std=c++17。
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_PREFIX_PATH "C:/msys64/mingw64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # 可执行文件输出到 bin 目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 静态库输出到 lib 目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 动态库输出到 lib 目录

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)
#find_package(Qt6 COMPONENTS Core Widgets Sql REQUIRED)

# FFMPeg相关模块
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
#   libavformat
#   libavcodec
#   libavutil
#   libswscale
# )
# 添加log4cpp头文件路径
# include_directories(${CMAKE_SOURCE_DIR}/External/log4cpp-master/include)
# 设置log4cpp库文件路径
#set(LOG4CPP_LIB ${CMAKE_SOURCE_DIR}/External/log4cpp-master/lib/debug/liblog4cpp.dll.a)
# set(LOG4CPP_LIB ${CMAKE_SOURCE_DIR}/External/log4cpp-master/lib/release/liblog4cpp.dll.a)

# 添加源文件
add_executable(SQLiteDemo
    src/main.cpp
    src/mainwindow.cpp
    src/editablemodel.cpp
    src/mainwindow.h
    src/editablemodel.h
)

target_link_libraries(SQLiteDemo PRIVATE Qt5::Widgets Qt5::Sql)
# 防止弹出命令行窗口
set_target_properties(SQLiteDemo PROPERTIES WIN32_EXECUTABLE ON)



