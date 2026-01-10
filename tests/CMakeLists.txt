cmake_minimum_required(VERSION 3.20)

# End-to-end assembler + sim tests

set(ASM_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/asm)
file(GLOB ASM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

if(ASM_FILES)
  add_custom_target(asm_outputs ALL)

  foreach(ASM_FILE ${ASM_FILES})
    get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)
    set(ASM_BIN ${ASM_OUTPUT_DIR}/${ASM_NAME}.bin)
    set(ASM_JSON ${ASM_OUTPUT_DIR}/${ASM_NAME}.json)

    add_custom_command(
      OUTPUT ${ASM_BIN} ${ASM_JSON}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ASM_OUTPUT_DIR}
      COMMAND $<TARGET_FILE:irata2_asm> ${ASM_FILE} ${ASM_BIN} ${ASM_JSON}
      DEPENDS ${ASM_FILE} irata2_asm
      COMMENT "Assembling ${ASM_NAME}.asm"
      VERBATIM
    )

    add_custom_target(asm_${ASM_NAME} DEPENDS ${ASM_BIN} ${ASM_JSON})
    add_dependencies(asm_outputs asm_${ASM_NAME})

    set(TEST_ARGS ${ASM_BIN})
    if(ASM_NAME STREQUAL "crs")
      set(TEST_ARGS --expect-crash ${ASM_BIN})
    endif()

    add_test(NAME asm_${ASM_NAME}
             COMMAND $<TARGET_FILE:irata2_run> ${TEST_ARGS})
  endforeach()
endif()
