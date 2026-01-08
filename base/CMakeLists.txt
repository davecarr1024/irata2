cmake_minimum_required(VERSION 3.20)

# Base Module - Fundamental types and utilities
# This module is self-contained and can be built independently

# Base library (interface, header-only)
add_library(irata2_base INTERFACE)
add_library(irata2::base ALIAS irata2_base)

target_include_directories(irata2_base INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Require C++20
target_compile_features(irata2_base INTERFACE cxx_std_20)

# Export configuration
install(TARGETS irata2_base
  EXPORT irata2BaseTargets
  INCLUDES DESTINATION include
)

install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT irata2BaseTargets
  FILE irata2BaseTargets.cmake
  NAMESPACE irata2::
  DESTINATION lib/cmake/irata2
)

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
