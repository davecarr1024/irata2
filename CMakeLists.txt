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

# Base module - fundamental types
add_library(irata2_base INTERFACE)
target_include_directories(irata2_base INTERFACE
  base/include
)

# HDL module - hardware definition language (structural metadata)
add_library(irata2_hdl
  hdl/src/cpu.cpp
)
target_include_directories(irata2_hdl PUBLIC
  hdl/include
)
target_link_libraries(irata2_hdl PUBLIC
  irata2_base
)

# Simulator module - runtime execution
add_library(irata2_sim
  sim/src/cpu.cpp
)
target_include_directories(irata2_sim PUBLIC
  sim/include
)
target_link_libraries(irata2_sim PUBLIC
  irata2_base
  irata2_hdl
)

# Microcode module (will be populated later)
add_library(irata2_microcode INTERFACE)
target_include_directories(irata2_microcode INTERFACE
  microcode/include
)
target_link_libraries(irata2_microcode INTERFACE
  irata2_base
  irata2_hdl
)

# Testing with GoogleTest (to be added)
# enable_testing()
# find_package(GTest REQUIRED)
# add_subdirectory(test)
