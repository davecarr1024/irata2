cmake_minimum_required(VERSION 3.20)

# Base module tests

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

# Test executable for base module
add_executable(base_tests
  byte_test.cpp
  word_test.cpp
  tick_phase_test.cpp
  log_test.cpp
)

target_link_libraries(base_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  irata2::base
)

target_compile_features(base_tests PRIVATE cxx_std_20)

gtest_discover_tests(base_tests)
