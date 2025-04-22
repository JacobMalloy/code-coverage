cmake_minimum_required(VERSION 3.13)
project(PrintBBInfoPass)

find_package(LLVM REQUIRED CONFIG)

add_library(PrintBBInfoPass MODULE PrintBBInfo.cpp)

set_target_properties(PrintBBInfoPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

target_include_directories(PrintBBInfoPass PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(PrintBBInfoPass PRIVATE LLVMCore LLVMSupport)

llvm_map_components_to_libnames(llvm_libs support core irreader)

