cmake_minimum_required(VERSION 3.16)#要求CMake的最低版本为3.16，确保功能兼容
project(cpp_lab LANGUAGES CXX)#项目名为cpp_lab，用C++(CXX)

set(CMAKE_CXX_STANDARD 17)#指定用C++标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)#若编译器不支持就报错
set(CMAKE_CXX_EXTENSIONS OFF)#关闭编译器特有的扩展，使用纯正的标准C++17，保证代码跨平台

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -g)

add_executable(cpp_lab src/main.cpp)#生成名为cpp_lab的可执行文件
target_include_directories(cpp_lab PRIVATE include)#告诉编译器去include找头文件(.h)
