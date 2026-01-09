cmake_minimum_required(VERSION 3.20)

# HDL module tests

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

# Test executable for hdl module
add_executable(hdl_tests
  bus_test.cpp
  byte_bus_test.cpp
  byte_register_test.cpp
  component_base_test.cpp
  component_test.cpp
  component_with_bus_test.cpp
  component_with_parent_test.cpp
  control_test.cpp
  counter_test.cpp
  cpu_test.cpp
  process_control_test.cpp
  read_control_test.cpp
  register_test.cpp
  traits_test.cpp
  word_bus_test.cpp
  word_register_test.cpp
  write_control_test.cpp
)

target_link_libraries(hdl_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  irata2::hdl
)

target_compile_features(hdl_tests PRIVATE cxx_std_20)

gtest_discover_tests(hdl_tests)

# Code coverage support
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(hdl_tests PRIVATE --coverage)
  target_link_options(hdl_tests PRIVATE --coverage)

  target_compile_options(irata2_hdl PRIVATE --coverage)
  target_link_options(irata2_hdl PRIVATE --coverage)
endif()
