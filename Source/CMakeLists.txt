add_subdirectory(Reai)

add_executable(main Main.c)
target_link_libraries(
  main PRIVATE
  reai
  toml
  ${SQLite3_LIBRARIES}
)
target_include_directories(
  main PRIVATE
  ${PROJECT_SOURCE_DIR}/Include
  ${CJSON_INCLUDE_DIRS}
  ${TOML_INCLUDE_DIRS}
  ${SQLite3_INCLUDE_DIRS}
)
