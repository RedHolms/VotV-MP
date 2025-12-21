foreach(TARGET IN LISTS ALL_TARGETS)
  target_precompile_headers(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Stdafx.hpp")
endforeach()

file(GLOB_RECURSE CLIENT_SOURCES "Client/*.cpp" "Client/*.hpp" "Client/*.c" "Client/*.h")
target_sources(Client PRIVATE ${CLIENT_SOURCES})

file(GLOB_RECURSE LAUNCHER_SOURCES "Launcher/*.cpp" "Launcher/*.hpp" "Launcher/*.c" "Launcher/*.h")
target_sources(Launcher PRIVATE ${LAUNCHER_SOURCES})
