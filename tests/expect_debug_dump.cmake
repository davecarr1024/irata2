if(NOT DEFINED RUNNER OR NOT DEFINED DEBUG_JSON OR NOT DEFINED CARTRIDGE)
  message(FATAL_ERROR "RUNNER, DEBUG_JSON, and CARTRIDGE must be set")
endif()

execute_process(
  COMMAND "${RUNNER}" --debug "${DEBUG_JSON}" "${CARTRIDGE}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout_output
  ERROR_VARIABLE stderr_output
)

set(full_output "${stdout_output}\n${stderr_output}")

if(result EQUAL 0)
  message(FATAL_ERROR "Expected irata2_run to fail for debug dump validation.")
endif()

if(NOT full_output MATCHES "Debug dump \\(crash\\)")
  message(FATAL_ERROR "Expected debug dump output to include crash header.")
endif()
