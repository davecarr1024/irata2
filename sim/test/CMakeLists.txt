cmake_minimum_required(VERSION 3.20)

# Simulator module tests

# Fetch GoogleTest if not already available
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# Test executable for sim module
add_executable(sim_tests
  alu_test.cpp
  bus_test.cpp
  control_test.cpp
  counter_test.cpp
  component_test.cpp
  controller_test.cpp
  cpu_debug_test.cpp
  cpu_test.cpp
  debug_dump_test.cpp
  debug_trace_test.cpp
  debug_symbols_test.cpp
  initialization_test.cpp
  memory_test.cpp
  register_test.cpp
  status_test.cpp
)

target_link_libraries(sim_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  irata2::sim
)

target_compile_features(sim_tests PRIVATE cxx_std_20)

gtest_discover_tests(sim_tests)

# Code coverage support
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(sim_tests PRIVATE --coverage)
  target_link_options(sim_tests PRIVATE --coverage)

  target_compile_options(irata2_sim PRIVATE --coverage)
  target_link_options(irata2_sim PRIVATE --coverage)
endif()
