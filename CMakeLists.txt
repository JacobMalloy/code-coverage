cmake_minimum_required(VERSION 3.13.4)

project(MyPass VERSION 0.1 LANGUAGES CXX)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_llvm_pass_plugin(MyPass MyPass.cpp)
