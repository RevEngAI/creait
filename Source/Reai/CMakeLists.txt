find_package(CURL REQUIRED)

file(GLOB_RECURSE LibReaiSRCS ${CMAKE_CURRENT_SOURCE_DIR} *.c)

# Reai Library
add_library(reai ${LibReaiSRCS})
target_link_libraries(reai PUBLIC cjson CURL::libcurl)
target_include_directories(reai PUBLIC ${PROJECT_SOURCE_DIR}/Include ${PROJECT_SOURCE_DIR}/Deps/cJSON)
set_target_properties(reai PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

# Add installation target for library
install(TARGETS reai DESTINATION lib)
