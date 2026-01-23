cmake_minimum_required(VERSION 3.20)

# Assembler Module - lexer/parser/encoder pipeline

add_library(irata2_assembler
  src/assembler.cpp
  src/cartridge.cpp
  src/lexer.cpp
  src/parser.cpp
  src/include_processor.cpp
)
add_library(irata2::assembler ALIAS irata2_assembler)

target_include_directories(irata2_assembler PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(irata2_assembler PUBLIC
  irata2::base
  irata2::isa
)

target_compile_features(irata2_assembler PUBLIC cxx_std_20)

add_executable(irata2_asm src/main.cpp)
target_link_libraries(irata2_asm PRIVATE irata2::assembler)
set_target_properties(irata2_asm PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

# Export configuration (optional, disabled by default for development)
option(ENABLE_INSTALL "Enable install targets" OFF)
if(ENABLE_INSTALL)
  install(TARGETS irata2_assembler irata2_asm
    EXPORT irata2AssemblerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
  )

  install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
  )

  install(EXPORT irata2AssemblerTargets
    FILE irata2AssemblerTargets.cmake
    NAMESPACE irata2::
    DESTINATION lib/cmake/irata2
  )
endif()

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
