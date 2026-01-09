cmake_minimum_required(VERSION 3.20)

# ISA Module - ISA definitions and code generation
# This module is self-contained and can be built independently

# Find Python for code generation
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Set up paths
set(ISA_YAML ${CMAKE_CURRENT_SOURCE_DIR}/instructions.yaml)
set(ISA_HEADER ${CMAKE_CURRENT_BINARY_DIR}/include/irata2/isa/isa.h)
set(ISA_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/generate_isa.py)

# Custom command to generate ISA header from YAML
add_custom_command(
  OUTPUT ${ISA_HEADER}
  COMMAND ${Python3_EXECUTABLE} ${ISA_GENERATOR} ${ISA_YAML} ${ISA_HEADER}
  DEPENDS ${ISA_YAML} ${ISA_GENERATOR}
  COMMENT "Generating ISA header from instructions.yaml"
  VERBATIM
)

# Custom target to ensure generation happens
add_custom_target(generate_isa ALL DEPENDS ${ISA_HEADER})

# ISA library (interface, header-only)
# Generated code is in the build directory
add_library(irata2_isa INTERFACE)
add_library(irata2::isa ALIAS irata2_isa)

target_include_directories(irata2_isa INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Ensure ISA is generated before using the library
add_dependencies(irata2_isa generate_isa)

# Optional: Build example usage program
option(BUILD_ISA_EXAMPLES "Build ISA module examples" OFF)

if(BUILD_ISA_EXAMPLES)
  add_executable(isa_example example_usage.cpp)
  target_link_libraries(isa_example PRIVATE irata2::isa)
  set_target_properties(isa_example PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
  )
endif()

# Export configuration
install(TARGETS irata2_isa
  EXPORT irata2IsaTargets
  INCLUDES DESTINATION include
)

install(FILES ${ISA_HEADER}
  DESTINATION include/irata2/isa
)

install(EXPORT irata2IsaTargets
  FILE irata2IsaTargets.cmake
  NAMESPACE irata2::
  DESTINATION lib/cmake/irata2
)

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
