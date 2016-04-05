if (WITH_JSON)
  add_definitions (-DWITH_JSON)
endif ()

add_subdirectory (src)

if (BUILD_TESTS)
  add_subdirectory (tests)
endif ()
