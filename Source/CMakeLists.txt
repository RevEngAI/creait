add_subdirectory(Reai)

add_executable(main Main.c)
target_link_libraries(
  main PRIVATE
  reai
  tomlc99
  ${SQLite3_LIBRARIES}
)
target_include_directories(
  main PRIVATE
  ${PROJECT_SOURCE_DIR}/Include
  ${PROJECT_SOURCE_DIR}/Deps/cJSON
  ${PROJECT_SOURCE_DIR}/Deps/tomlc99
  ${SQLite3_INCLUDE_DIRS}
)
