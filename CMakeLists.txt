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

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
  set(MSVC_LIKE_CC ON)
else()
  message(FATAL_ERROR "Unsupported compiler (${CMAKE_CXX_COMPILER_ID}, ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_EXAMPLES OFF)
set(BUILD_SHARED_LIBS OFF)
set(FMT_MASTER_PROJECT OFF CACHE BOOL "" FORCE)

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

# Common source files (shared between client and servers)
file(GLOB_RECURSE SOURCES "Source/Common/*" "Source/Logging/*" "Source/Network/*")

# Client-only sources
file(GLOB_RECURSE CLIENT_SOURCES "Client/*")
set(CLIENT_SOURCES "Resources/Client.rc" ${SOURCES} ${CLIENT_SOURCES})

# Launcher-only sources
file(GLOB_RECURSE LAUNCHER_SOURCES "Launcher/*")
set(LAUNCHER_SOURCES "Resources/Launcher.rc" ${LAUNCHER_SOURCES})

# Register targets
add_library(Client MODULE EXCLUDE_FROM_ALL ${CLIENT_SOURCES})
add_executable(Launcher WIN32 EXCLUDE_FROM_ALL ${LAUNCHER_SOURCES})
set(ALL_TARGETS Client Launcher)

# Apply shared options to all targets
foreach(TARGET IN LISTS ALL_TARGETS)
  # Pre-compile and auto-include Stdafx.hpp
  target_sources(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Stdafx.hpp")
  target_precompile_headers(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Stdafx.hpp")
  if(MSVC_LIKE_CC)
    target_compile_options(${TARGET} PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/Source/Stdafx.hpp")
  endif()

  # Base include directory
  target_include_directories(${TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
  )

  # For logging system
  target_compile_definitions(${TARGET} PRIVATE
    _PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/Source/"
  )

  # Link with fmt library
  target_link_libraries(${TARGET} PRIVATE fmt::fmt)
endforeach()

target_link_libraries(Launcher PRIVATE
  Shlwapi.lib
)
