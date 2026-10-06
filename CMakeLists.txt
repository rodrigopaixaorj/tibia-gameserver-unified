cmake_minimum_required(VERSION 3.15)
project(tibia-server LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    src
    src/compat
    src/crypto
    src/database
    src/login
    src/game
)

file(GLOB_RECURSE SERVER_SOURCES
    src/compat/*.cc
    src/crypto/*.cc
    src/database/*.c
    src/database/*.cc
    src/login/*.cc
    src/game/*.cc
    src/main.cc
)

add_executable(tibia-server ${SERVER_SOURCES})

if(WIN32)
    target_link_libraries(tibia-server PRIVATE ws2_32 mpir)
    target_compile_definitions(tibia-server PRIVATE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN OS_WINDOWS=1)
else()
    target_link_libraries(tibia-server PRIVATE pthread dl gmp)
    target_compile_definitions(tibia-server PRIVATE OS_LINUX=1)
endif()
