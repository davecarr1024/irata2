cmake_minimum_required(VERSION 3.20)

# Microcode module tests

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

add_executable(microcode_tests
  builder_test.cpp
  bus_validator_test.cpp
  compiler_conflict_test.cpp
  compiler_test.cpp
  control_conflict_validator_test.cpp
  control_encoder_test.cpp
  cpu_path_resolver_test.cpp
  decoder_test.cpp
  empty_step_optimizer_test.cpp
  fetch_transformer_test.cpp
  fetch_validator_test.cpp
  isa_coverage_validator_test.cpp
  instruction_encoder_test.cpp
  phase_ordering_validator_test.cpp
  sequence_transformer_test.cpp
  sequence_validator_test.cpp
  stage_validator_test.cpp
  status_encoder_test.cpp
  status_validator_test.cpp
)

target_link_libraries(microcode_tests PRIVATE
  GTest::gtest_main
  GTest::gmock
  irata2::microcode
)

target_compile_features(microcode_tests PRIVATE cxx_std_20)

gtest_discover_tests(microcode_tests)
