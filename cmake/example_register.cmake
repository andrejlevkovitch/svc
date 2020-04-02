macro(example_register EXAMPLE_NAME EXAMPLE_FILE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
  target_compile_features(${EXAMPLE_NAME} PRIVATE cxx_std_17)
  target_link_libraries(${EXAMPLE_NAME} PRIVATE
    ${PROJECT_NAME}
    )
endmacro()
