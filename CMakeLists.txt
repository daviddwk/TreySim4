cmake_minimum_required(VERSION 3.18.1)
project(game)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-03")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("lib")
add_subdirectory(lib/Eendgine)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")

add_executable(game ${SOURCES})
set_target_properties(
    game 
    PROPERTIES 
        OUTPUT_NAME "game"
        SUFFIX ".out"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
target_link_libraries(game Eendgine)
target_compile_options(game PRIVATE -Wall)
