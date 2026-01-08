cmake_minimum_required(VERSION 3.20)
project(irata2 VERSION 0.1.0 LANGUAGES CXX)

# C++20 required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add subdirectories for each module
# Modules are built in dependency order

# 1. Base module - no dependencies
add_subdirectory(base)

# 2. HDL module - depends on base
add_subdirectory(hdl)

# 3. ASM module - depends on base (generates ISA from YAML)
add_subdirectory(asm)

# 4. Simulator module - depends on base, hdl
add_subdirectory(sim)

# 5. Microcode module - depends on base, hdl, asm
add_subdirectory(microcode)

# Testing with GoogleTest (to be added)
# enable_testing()
# find_package(GTest REQUIRED)
# add_subdirectory(test)
