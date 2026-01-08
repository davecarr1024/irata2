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
  component_test.cpp
  cpu_test.cpp
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
