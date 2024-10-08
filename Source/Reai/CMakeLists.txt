find_package(CURL)

file(GLOB_RECURSE CREAIT_SRCS ${CMAKE_CURRENT_SOURCE_DIR} *.c)

# Dependencies
# Libraries dependents need to link to to use REAI
set(
  CREAIT_LINK_LIBRARIES
  cjson CURL::libcurl toml
  CACHE STRING "Libraries to link to in order to use creait"
)

# Reai Library
add_library(reai ${CREAIT_SRCS})
target_link_libraries(reai PUBLIC ${CREAIT_LINK_LIBRARIES})
target_link_directories(reai PUBLIC ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
target_include_directories(
  reai
  PUBLIC
  ${PROJECT_SOURCE_DIR}/Include
  ${CJSON_INCLUDE_DIRS}
  ${TOML_INCLUDE_DIRS}
)
set_target_properties(
  reai
  PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
)

# Add installation target for library
install(TARGETS reai DESTINATION lib)
