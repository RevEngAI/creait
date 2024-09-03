add_subdirectory(Reai)

add_executable(main Main.c)

# In the end, all built libraries are stored in "build/lib"
find_library(libreai NAMES reai HINTS ${CMAKE_BINARY_DIR}/lib REQUIRED)
find_library(libtoml NAMES toml HINTS ${CMAKE_BINARY_DIR}/lib REQUIRED)
find_library(libcjson NAMES cjson HINTS ${CMAKE_BINARY_DIR}/lib REQUIRED)

target_link_libraries(
  main PRIVATE
  ${libreai}
  ${libtoml}
  ${libcjson}
  ${SQLite3_LIBRARIES}
  ${CURL_LIBRARIES}
)
target_include_directories(
  main PRIVATE
  ${PROJECT_SOURCE_DIR}/Include
  ${CJSON_INCLUDE_DIRS}
  ${TOML_INCLUDE_DIRS}
  ${SQLite3_INCLUDE_DIRS}
)
