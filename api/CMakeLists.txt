set (API_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "API_INCLUDE_DIR")

add_subdirectory (src)
if (BUILD_EXAMPLES)
add_subdirectory (examples)
endif ()

if (BUILD_TESTS)
  add_subdirectory (tests)
endif ()

install (DIRECTORY include/awa
  DESTINATION include
)

