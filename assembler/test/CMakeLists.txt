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
  demo_programs_test.cpp
)

target_link_libraries(assembler_tests PRIVATE
  GTest::gtest_main
  irata2::assembler
)

target_compile_definitions(assembler_tests PRIVATE
  IRATA2_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_compile_features(assembler_tests PRIVATE cxx_std_20)

gtest_discover_tests(assembler_tests
  PROPERTIES TIMEOUT 30
)

# Integration tests - run assembled programs end-to-end
# These tests run actual CPU simulations and could hang if microcode is buggy
add_executable(integration_tests
  graphics_test_helpers.cpp
  integration/graphics_test.cpp
  integration/system_test.cpp
  integration/load_immediate_test.cpp
  integration/arithmetic_immediate_test.cpp
  integration/logic_immediate_test.cpp
  integration/shift_accumulator_test.cpp
  integration/register_transfer_test.cpp
  integration/absolute_test.cpp
  integration/branch_test.cpp
  integration/inc_dec_test.cpp
  integration/zero_page_test.cpp
  integration/zero_page_indexed_test.cpp
  integration/asteroids_modules_test.cpp
)

target_link_libraries(integration_tests PRIVATE
  GTest::gtest_main
  irata2::assembler
  irata2::sim
  irata2::base
)

target_compile_features(integration_tests PRIVATE cxx_std_20)

target_include_directories(integration_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(integration_tests
  PROPERTIES TIMEOUT 60  # Longer timeout for integration tests
)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(integration_tests PRIVATE --coverage)
  target_link_options(integration_tests PRIVATE --coverage)
endif()

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
