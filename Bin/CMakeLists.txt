cmake_minimum_required(VERSION 3.5)

# Create the ReaiDiff executable
add_executable(ReaiDiff ReaiDiff.c)

# Link against the CREAIT library
target_link_libraries(ReaiDiff 
    PRIVATE 
    reai
    ${CURL_LIBRARIES}
)

# Include directories for headers
target_include_directories(ReaiDiff 
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/../Include"
    "${CMAKE_PREFIX_PATH}/include"
)

# Set output directory
set_target_properties(ReaiDiff PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the executable
install(TARGETS ReaiDiff DESTINATION bin) 