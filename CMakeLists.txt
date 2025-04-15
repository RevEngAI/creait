cmake_minimum_required(VERSION 3.12)

project(CREAIT LANGUAGES C VERSION 0.1.1)
include(FetchContent)
include(ExternalProject)
include(CTest)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# set output directories of binary and library files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIRBARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# generate a compile_commands.json for LSP clients
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add dependencies to project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
include(Dependencies)
find_package(CURL REQUIRED)

link_directories(
  "${CMAKE_PREFIX_PATH}/lib"
  "${CMAKE_PREFIX_PATH}/bin"
)

include_directories(
  "${CMAKE_PREFIX_PATH}/include"
)

# Add library source
add_subdirectory(Source)
add_subdirectory(Test)

# Add installation target for include files
install(DIRECTORY Include/Reai DESTINATION include)
