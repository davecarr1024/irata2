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

# ASM module - ISA definitions (generated from YAML)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(ISA_YAML ${CMAKE_CURRENT_SOURCE_DIR}/asm/instructions.yaml)
set(ISA_HEADER ${CMAKE_CURRENT_BINARY_DIR}/asm/include/irata2/asm/isa.h)
set(ISA_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_isa.py)

# Custom command to generate ISA header from YAML
add_custom_command(
  OUTPUT ${ISA_HEADER}
  COMMAND ${Python3_EXECUTABLE} ${ISA_GENERATOR} ${ISA_YAML} ${ISA_HEADER}
  DEPENDS ${ISA_YAML} ${ISA_GENERATOR}
  COMMENT "Generating ISA header from instructions.yaml"
  VERBATIM
)

# Custom target to ensure generation happens
add_custom_target(generate_isa DEPENDS ${ISA_HEADER})

# ASM library (interface, header-only)
add_library(irata2_asm INTERFACE)
target_include_directories(irata2_asm INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}/asm/include
)
target_link_libraries(irata2_asm INTERFACE
  irata2_base
)

# Ensure ISA is generated before using the library
add_dependencies(irata2_asm generate_isa)

# Microcode module (will be populated later)
add_library(irata2_microcode INTERFACE)
target_include_directories(irata2_microcode INTERFACE
  microcode/include
)
target_link_libraries(irata2_microcode INTERFACE
  irata2_base
  irata2_hdl
  irata2_asm
)

# Testing with GoogleTest (to be added)
# enable_testing()
# find_package(GTest REQUIRED)
# add_subdirectory(test)
