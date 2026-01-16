cmake_minimum_required(VERSION 3.20)

# Simulator Module - Runtime execution
# This module is self-contained and can be built independently

# Simulator library (compiled)
add_library(irata2_sim
  src/alu/alu.cpp
  src/cartridge.cpp
  src/cpu.cpp
  src/cpu_debug_symbols.cpp
  src/controller/control_encoder.cpp
  src/controller/controller.cpp
  src/debug_dump.cpp
  src/initialization.cpp
  src/memory/memory.cpp
  src/memory/memory_address_register.cpp
  src/memory/module.cpp
  src/memory/region.cpp
  src/debug_symbols.cpp
  src/status.cpp
)
add_library(irata2::sim ALIAS irata2_sim)

target_include_directories(irata2_sim PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Depend on base and hdl modules
target_link_libraries(irata2_sim PUBLIC
  irata2::base
  irata2::hdl
  irata2::microcode
)

# Require C++20
target_compile_features(irata2_sim PUBLIC cxx_std_20)

# Simulator runner
add_executable(irata2_run
  src/run.cpp
)
target_link_libraries(irata2_run PRIVATE irata2::sim)
target_compile_features(irata2_run PRIVATE cxx_std_20)

# Export configuration (optional, disabled by default for development)
option(ENABLE_INSTALL "Enable install targets" OFF)
if(ENABLE_INSTALL)
  install(TARGETS irata2_sim
    EXPORT irata2SimTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
  )

  install(TARGETS irata2_run
    RUNTIME DESTINATION bin
  )

  install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
  )

  install(EXPORT irata2SimTargets
    FILE irata2SimTargets.cmake
    NAMESPACE irata2::
    DESTINATION lib/cmake/irata2
  )
endif()

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
