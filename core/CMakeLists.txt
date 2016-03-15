if (WITH_JSON)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_JSON")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_JSON")
endif ()

add_subdirectory (src)

if (BUILD_TESTS)
  add_subdirectory (tests)
endif ()
