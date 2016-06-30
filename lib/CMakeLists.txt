add_subdirectory (b64)
add_subdirectory (hmac)
if (WITH_LIBCOAP)
  add_subdirectory (libcoap)
endif ()
add_subdirectory (xml)

if (WITH_JSON)
  add_subdirectory (jsmn)
endif ()

if (WITH_TINYDTLS)
  add_subdirectory (tinydtls)
endif ()

if (BUILD_TESTS)
  # gtest is CMake-enabled
  #  defines libraries 'gtest' and 'gtest-main'
  add_subdirectory (gtest)
#  set (GTEST_INCLUDE_DIR ${gtest_SOURCE_DIR}/include CACHE INTERNAL "GTEST_INCLUDE_DIR")  
endif ()
