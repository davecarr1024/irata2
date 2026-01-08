cmake_minimum_required(VERSION 3.20)

# ASM module tests

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

# Test executable for asm module
add_executable(asm_tests
  isa_test.cpp
)

target_link_libraries(asm_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  irata2::asm
)

# Ensure ISA is generated before building tests
add_dependencies(asm_tests generate_isa)

target_compile_features(asm_tests PRIVATE cxx_std_20)

gtest_discover_tests(asm_tests)
