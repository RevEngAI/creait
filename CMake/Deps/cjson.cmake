# Include the PkgConfig module
find_package(PkgConfig REQUIRED)

# Use pkg-config to find the package
pkg_check_modules(CJSON libcjson)

if(${CJSON_FOUND})
    message(STATUS "cJSON found.")
else()
    message(STATUS "cJSON not found. Fetching and building...")

    FetchContent_Declare(
        CJSON
        URL     "https://github.com/DaveGamble/cJSON/archive/refs/tags/v1.7.18.tar.gz"
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(CJSON)

    # Access and print the source directory
    FetchContent_GetProperties(CJSON SOURCE_DIR CJSON_SOURCE_DIR)

    set(CJSON_INCLUDE_DIRS ${CJSON_SOURCE_DIR} CACHE PATH "Include directory for cJSON library.")
endif()
