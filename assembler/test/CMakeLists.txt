cmake_minimum_required(VERSION 3.20)

# Assembler module tests

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

add_executable(assembler_tests
  assembler_test.cpp
)

target_link_libraries(assembler_tests PRIVATE
  GTest::gtest_main
  irata2::assembler
)

target_compile_features(assembler_tests PRIVATE cxx_std_20)

gtest_discover_tests(assembler_tests)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME debug_schema_validator
    COMMAND ${Python3_EXECUTABLE}
      ${CMAKE_SOURCE_DIR}/scripts/validate_debug_schema.py
      --schema ${CMAKE_SOURCE_DIR}/assembler/debug_schema.json
      --assembler $<TARGET_FILE:irata2_asm>
      --source ${CMAKE_SOURCE_DIR}/tests/lda.asm
  )
endif()

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(assembler_tests PRIVATE --coverage)
  target_link_options(assembler_tests PRIVATE --coverage)

  target_compile_options(irata2_assembler PRIVATE --coverage)
  target_link_options(irata2_assembler PRIVATE --coverage)
endif()
