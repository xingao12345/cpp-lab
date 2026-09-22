cmake_minimum_required(VERSION 3.16)
project(cpp_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -g)

add_executable(cpp_lab src/main.cpp)
target_include_directories(cpp_lab PRIVATE include)
