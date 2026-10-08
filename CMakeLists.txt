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
    # Check vcpkg default include & lib paths if present
    if(EXISTS "D:/vcpkg/installed/x64-windows/include")
        target_include_directories(tibia-server PRIVATE
            "D:/vcpkg/installed/x64-windows/include"
            "D:/vcpkg/installed/x64-windows/include/mysql"
        )
        target_link_directories(tibia-server PRIVATE
            "D:/vcpkg/installed/x64-windows/lib"
        )
    elseif(EXISTS "C:/vcpkg/installed/x64-windows/include")
        target_include_directories(tibia-server PRIVATE
            "C:/vcpkg/installed/x64-windows/include"
            "C:/vcpkg/installed/x64-windows/include/mysql"
        )
        target_link_directories(tibia-server PRIVATE
            "C:/vcpkg/installed/x64-windows/lib"
        )
    endif()

    target_link_libraries(tibia-server PRIVATE ws2_32 mpir libmariadb)
    target_compile_definitions(tibia-server PRIVATE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN OS_WINDOWS=1)
else()
    find_package(MySQL QUIET)
    if(MYSQL_FOUND)
        target_include_directories(tibia-server PRIVATE ${MYSQL_INCLUDE_DIR})
        target_link_libraries(tibia-server PRIVATE pthread dl gmp ${MYSQL_CLIENT_LIBS})
    else()
        target_link_libraries(tibia-server PRIVATE pthread dl gmp mariadb)
    endif()
    target_compile_definitions(tibia-server PRIVATE OS_LINUX=1)
endif()
