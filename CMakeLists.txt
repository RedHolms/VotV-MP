cmake_minimum_required(VERSION 3.18)

project("VotV-Multiplayer" VERSION 0.0.1.0)

add_compile_definitions(
  VOTVMP_VERSION="${PROJECT_VERSION}"
  VOTVMP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  VOTVMP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  VOTVMP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  VOTVMP_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options("/utf-8")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX)
  add_link_options("/OPT:REF,ICF")
endif()

include(FetchContent)

FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        b18ece7d71a5a0c6c6aba4297b3e828daa410fc7
)

FetchContent_MakeAvailable(fmtlib)

add_executable(Launcher WIN32 EXCLUDE_FROM_ALL)
add_library(Client MODULE EXCLUDE_FROM_ALL)
set(ALL_TARGETS Launcher Client)

add_subdirectory("Source")
add_subdirectory("Resources")

target_link_libraries(Launcher PRIVATE
  Shlwapi.lib
)
