cmake_minimum_required(VERSION 2.8.2)
 
project(googletest NONE)

if (DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_EXTRA_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

Include (ExternalProject)
ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG ff07a5de0e81580547f1685e101194ed1a4fcd56
    CMAKE_ARGS ${CMAKE_EXTRA_ARGS}
    SOURCE_DIR     "${CMAKE_BINARY_DIR}/gtest-src"
    BINARY_DIR     "${CMAKE_BINARY_DIR}/gtest-build"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
)

set (GTEST_INCLUDE_DIR ${CMAKE_BINARY_DIR}/gtest-src/googletest/include CACHE INTERNAL "GTEST_INCLUDE_DIR")

add_library(gtest STATIC IMPORTED GLOBAL)
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/gtest-build/googlemock/gtest/libgtest.a")
#set_target_properties(gtest PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR})

