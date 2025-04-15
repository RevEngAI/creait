pkg_check_modules(CJSON REQUIRED libcjson)

add_executable(reai_test Test.c MockApi.c)
target_link_libraries(reai_test reai ${CJSON_LIBRARIES})
target_link_directories(reai_test PRIVATE ${CJSON_LIBRARY_DIRS})

add_test(NAME ReaiTest COMMAND reai_test)
