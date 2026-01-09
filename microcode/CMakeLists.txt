cmake_minimum_required(VERSION 3.20)

# Microcode Module - DSL, IR, compiler, encoder
# This module is self-contained and can be built independently

# Microcode library
add_library(irata2_microcode
  src/compiler/compiler.cpp
  src/compiler/fetch_transformer.cpp
  src/compiler/fetch_validator.cpp
  src/compiler/isa_coverage_validator.cpp
  src/compiler/sequence_transformer.cpp
  src/compiler/sequence_validator.cpp
  src/encoder/control_encoder.cpp
  src/encoder/instruction_encoder.cpp
  src/encoder/status_encoder.cpp
  src/ir/builder.cpp
)
add_library(irata2::microcode ALIAS irata2_microcode)

target_include_directories(irata2_microcode PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Depend on base, hdl, and isa modules
target_link_libraries(irata2_microcode PUBLIC
  irata2::base
  irata2::hdl
  irata2::isa
)

# Require C++20
target_compile_features(irata2_microcode PUBLIC cxx_std_20)

# Export configuration
install(TARGETS irata2_microcode
  EXPORT irata2MicrocodeTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT irata2MicrocodeTargets
  FILE irata2MicrocodeTargets.cmake
  NAMESPACE irata2::
  DESTINATION lib/cmake/irata2
)

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
