cmake_minimum_required(VERSION 3.10)
project(Umbrella VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Source files
set(SOURCES
    src/umbrella.cpp
    src/compiler/lexer.cpp
    src/compiler/parser.cpp
    src/compiler/ast.cpp
    src/compiler/codegen.cpp
    src/runtime/runtime.cpp
    src/runtime/advanced.cpp
)

# Main compiler executable
add_executable(umbrella ${SOURCES})

# Link libraries
find_package(Threads REQUIRED)
target_link_libraries(umbrella Threads::Threads)

# Set C++ standard
target_compile_features(umbrella PRIVATE cxx_std_17)

# Include directories
target_include_directories(umbrella PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Installation
install(TARGETS umbrella DESTINATION bin)

# Print build information
message(STATUS "Umbrella Programming Language Compiler")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
