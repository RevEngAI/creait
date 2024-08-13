# Include the PkgConfig module
find_package(PkgConfig REQUIRED)

# Use pkg-config to find the package
pkg_check_modules(LIBTOML libtoml)

if(${LIBTOML_FOUND})
    message(STATUS "tomlc99 found.")
else()
    message(STATUS "tomlc99 not found. Fetching and building...")

    FetchContent_Declare(
        TOMLC99
        URL     "https://github.com/brightprogrammer/tomlc99/archive/refs/tags/v1.tar.gz"
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(TOMLC99)

    # Access and print the source directory
    FetchContent_GetProperties(TOMLC99 SOURCE_DIR TOMLC99_SOURCE_DIR)
    set(TOML_INCLUDE_DIRS ${TOMLC99_SOURCE_DIR} CACHE PATH "Include directory for libtoml.")
endif()
