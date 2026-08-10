cmake_minimum_required(VERSION 3.20)
project(Aqua LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Core library
add_library(AquaCore
        src/Name.cpp
        src/NameRegistry.cpp
        src/PropertyInfo.cpp
        src/FunctionInfo.cpp
        src/TypeInfo.cpp
        src/TypeRegistry.cpp
        src/Object.cpp
)
target_include_directories(AquaCore PUBLIC ${PROJECT_SOURCE_DIR}/src)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE AquaCore)

# Testing
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(AquaTests
        tests/SceneEntity.cpp
        tests/Player.cpp
        tests/NameTest.cpp
        tests/PropertyInfoTest.cpp
        tests/FunctionInfoTest.cpp
        tests/TypeInfoTest.cpp
)
target_link_libraries(AquaTests PRIVATE AquaCore GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(AquaTests)
