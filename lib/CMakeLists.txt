add_subdirectory (b64)
add_subdirectory (hmac)
add_subdirectory (libcoap)
add_subdirectory (xml)

if (WITH_JSON)
  add_subdirectory (jsmn)
endif ()

if (BUILD_TESTS)
  # gtest is CMake-enabled
  #  defines libraries 'gtest' and 'gtest-main'
  add_subdirectory (gtest)
#  set (GTEST_INCLUDE_DIR ${gtest_SOURCE_DIR}/include CACHE INTERNAL "GTEST_INCLUDE_DIR")  
endif ()
