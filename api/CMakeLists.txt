set (API_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "API_INCLUDE_DIR")

add_subdirectory (src)
add_subdirectory (examples)

if (BUILD_TESTS)
  add_subdirectory (tests)
endif ()

install (DIRECTORY include/awa
  DESTINATION include
)

