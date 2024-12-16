add_executable(reai_test Test.c MockApi.c)
target_link_libraries(reai_test reai)

add_test(NAME ReaiTest COMMAND reai_test)
