cmake_minimum_required(VERSION 3.20)

# HDL Module - Hardware Definition Language (structural metadata)
# This module is self-contained and can be built independently

# HDL library (compiled)
add_library(irata2_hdl
  src/cpu.cpp
)
add_library(irata2::hdl ALIAS irata2_hdl)

target_include_directories(irata2_hdl PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Depend on base module
target_link_libraries(irata2_hdl PUBLIC
  irata2::base
)

# Require C++20
target_compile_features(irata2_hdl PUBLIC cxx_std_20)

# Export configuration
install(TARGETS irata2_hdl
  EXPORT irata2HdlTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT irata2HdlTargets
  FILE irata2HdlTargets.cmake
  NAMESPACE irata2::
  DESTINATION lib/cmake/irata2
)
