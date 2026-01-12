cmake_minimum_required(VERSION 3.20)

# HDL Module - Hardware Definition Language (structural metadata)
# This module is self-contained and can be built independently

# HDL library (compiled)
add_library(irata2_hdl
  src/bus.cpp
  src/byte_bus.cpp
  src/byte_counter.cpp
  src/byte_register.cpp
  src/component.cpp
  src/component_base.cpp
  src/component_with_bus.cpp
  src/component_with_parent.cpp
  src/control.cpp
  src/controller.cpp
  src/counter.cpp
  src/cpu.cpp
  src/process_control.cpp
  src/read_control.cpp
  src/register.cpp
  src/status.cpp
  src/word_bus.cpp
  src/word_register.cpp
  src/write_control.cpp
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

# Export configuration (optional, disabled by default for development)
option(ENABLE_INSTALL "Enable install targets" OFF)
if(ENABLE_INSTALL)
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
endif()

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
