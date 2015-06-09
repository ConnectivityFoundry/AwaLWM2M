add_subdirectory (b64)
add_subdirectory (hmac)
add_subdirectory (libcoap)
add_subdirectory (xml)
add_subdirectory (jsmn)

if (BUILD_TESTS)
  # gtest is CMake-enabled
  #  defines libraries 'gtest' and 'gtest-main'
  add_subdirectory (gtest)
#  set (GTEST_INCLUDE_DIR ${gtest_SOURCE_DIR}/include CACHE INTERNAL "GTEST_INCLUDE_DIR")  
endif ()
