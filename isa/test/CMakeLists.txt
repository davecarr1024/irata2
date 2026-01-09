cmake_minimum_required(VERSION 3.20)

# ISA module tests

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

# Test executable for ISA module
add_executable(isa_tests
  isa_test.cpp
)

target_link_libraries(isa_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  irata2::isa
)

# Ensure ISA is generated before building tests
add_dependencies(isa_tests generate_isa)

target_compile_features(isa_tests PRIVATE cxx_std_20)

gtest_discover_tests(isa_tests)
