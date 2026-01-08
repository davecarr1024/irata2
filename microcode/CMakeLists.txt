cmake_minimum_required(VERSION 3.20)

# Microcode Module - DSL, IR, compiler, encoder
# This module is self-contained and can be built independently

# Microcode library (interface, will add sources later)
add_library(irata2_microcode INTERFACE)
add_library(irata2::microcode ALIAS irata2_microcode)

target_include_directories(irata2_microcode INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Depend on base, hdl, and asm modules
target_link_libraries(irata2_microcode INTERFACE
  irata2::base
  irata2::hdl
  irata2::asm
)

# Require C++20
target_compile_features(irata2_microcode INTERFACE cxx_std_20)

# Export configuration
install(TARGETS irata2_microcode
  EXPORT irata2MicrocodeTargets
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
