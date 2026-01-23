cmake_minimum_required(VERSION 3.20)

if(NOT IRATA2_ENABLE_SDL)
  return()
endif()

find_package(SDL2 REQUIRED)

add_library(irata2_frontend
  src/demo_runner.cpp
  src/sdl_backend.cpp
)
add_library(irata2::frontend ALIAS irata2_frontend)

target_include_directories(irata2_frontend PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(TARGET SDL2::SDL2)
  set(IRATA2_SDL_TARGET SDL2::SDL2)
elseif(TARGET SDL2::SDL2-static)
  set(IRATA2_SDL_TARGET SDL2::SDL2-static)
else()
  set(IRATA2_SDL_TARGET SDL2::SDL2)
endif()

target_include_directories(irata2_frontend PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(irata2_frontend PUBLIC
  irata2::sim
  ${IRATA2_SDL_TARGET}
)

add_executable(irata2_demo
  src/main.cpp
)
target_link_libraries(irata2_demo PRIVATE irata2::frontend)
target_compile_features(irata2_demo PRIVATE cxx_std_20)
