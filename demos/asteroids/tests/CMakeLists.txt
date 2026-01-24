cmake_minimum_required(VERSION 3.20)

# Asteroids module unit tests
# Auto-discovers and runs all .asm test files in this directory

set(ASM_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/asm)
file(GLOB ASM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

if(ASM_FILES)
  add_custom_target(asteroids_asm_outputs ALL)

  foreach(ASM_FILE ${ASM_FILES})
    get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)
    set(ASM_BIN ${ASM_OUTPUT_DIR}/${ASM_NAME}.bin)
    set(ASM_JSON ${ASM_OUTPUT_DIR}/${ASM_NAME}.json)

    add_custom_command(
      OUTPUT ${ASM_BIN} ${ASM_JSON}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ASM_OUTPUT_DIR}
      COMMAND $<TARGET_FILE:irata2_asm>
        ${ASM_FILE} ${ASM_BIN} ${ASM_JSON}
      DEPENDS ${ASM_FILE} irata2_asm
      COMMENT "Assembling asteroids test: ${ASM_NAME}.asm"
      VERBATIM
    )

    add_custom_target(asteroids_asm_${ASM_NAME} DEPENDS ${ASM_BIN} ${ASM_JSON})
    add_dependencies(asteroids_asm_outputs asteroids_asm_${ASM_NAME})

    # Default max cycles to prevent infinite loops
    set(MAX_CYCLES 100000)
    set(TEST_ARGS --max-cycles ${MAX_CYCLES} --debug ${ASM_JSON} ${ASM_BIN})

    add_test(NAME asteroids_${ASM_NAME}
             COMMAND $<TARGET_FILE:irata2_run> ${TEST_ARGS})
    set_tests_properties(asteroids_${ASM_NAME} PROPERTIES
      TIMEOUT 30
      LABELS "asteroids"
    )
  endforeach()
endif()
