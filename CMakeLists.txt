cmake_minimum_required(VERSION 3.20)
project(irata2 VERSION 0.1.0 LANGUAGES CXX)

# C++20 required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencies via FetchContent
include(FetchContent)

# fmt library for string formatting (portable alternative to std::format)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

# Abseil library for logging and utilities
FetchContent_Declare(
  absl
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG 20240116.2
)
set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_MakeAvailable(absl)

# Code coverage support
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
  else()
    message(WARNING "Code coverage not supported for this compiler")
  endif()
endif()

# Testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  enable_testing()
endif()

# Documentation generation with Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile)
  set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/docs)

  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
  message(STATUS "Doxygen found: documentation target 'docs' available")
else()
  message(STATUS "Doxygen not found: documentation target unavailable")
endif()

# Add subdirectories for each module
# Modules are built in dependency order

# 1. Base module - no dependencies
add_subdirectory(base)

# 2. HDL module - depends on base
add_subdirectory(hdl)

# 3. ISA module - depends on base (generates ISA from YAML)
add_subdirectory(isa)

# 4. Assembler module - depends on base, isa
add_subdirectory(assembler)

# 5. Simulator module - depends on base, hdl
add_subdirectory(sim)

# 6. Microcode module - depends on base, hdl, asm
add_subdirectory(microcode)

# 7. End-to-end tests (assembler + sim)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
