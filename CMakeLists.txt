cmake_minimum_required(VERSION 3.12)

project(CREAIT LANGUAGES C VERSION 0.1.1)
include(FetchContent)
include(ExternalProject)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# set output directories of binary and library files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIRBARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wno-error)

# generate a compile_commands.json for LSP clients
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add dependencies to project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(Dependencies)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

# Add library source
add_subdirectory(Source)

# Add installation target for include files
install(DIRECTORY Include/Reai DESTINATION include)
