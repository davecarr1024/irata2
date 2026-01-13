# Test script for log level configuration

cmake_minimum_required(VERSION 3.20)

# Test 1: --log-level error should suppress INFO logs
execute_process(
  COMMAND ${RUNNER} --log-level error ${CARTRIDGE}
  OUTPUT_VARIABLE OUTPUT_ERROR
  ERROR_VARIABLE STDERR_ERROR
  RESULT_VARIABLE RESULT_ERROR
)

if(STDERR_ERROR MATCHES "sim.start")
  message(FATAL_ERROR "Test failed: --log-level error should suppress INFO logs")
endif()

# Test 2: --log-level info should show INFO logs
execute_process(
  COMMAND ${RUNNER} --log-level info ${CARTRIDGE}
  OUTPUT_VARIABLE OUTPUT_INFO
  ERROR_VARIABLE STDERR_INFO
  RESULT_VARIABLE RESULT_INFO
)

if(NOT STDERR_INFO MATCHES "sim.start")
  message(FATAL_ERROR "Test failed: --log-level info should show INFO logs")
endif()

if(NOT STDERR_INFO MATCHES "sim.halt")
  message(FATAL_ERROR "Test failed: --log-level info should show sim.halt")
endif()

# Test 3: Environment variable IRATA2_LOG_LEVEL should work
set(ENV{IRATA2_LOG_LEVEL} "warning")
execute_process(
  COMMAND ${RUNNER} ${CARTRIDGE}
  OUTPUT_VARIABLE OUTPUT_ENV
  ERROR_VARIABLE STDERR_ENV
  RESULT_VARIABLE RESULT_ENV
)

if(STDERR_ENV MATCHES "sim.start")
  message(FATAL_ERROR "Test failed: IRATA2_LOG_LEVEL=warning should suppress INFO logs")
endif()

message(STATUS "All log level tests passed")
