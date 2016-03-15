cmake_minimum_required(VERSION 2.8.2)

project(libjsmn NONE)

if (DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_EXTRA_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

Include (ExternalProject)
ExternalProject_Add(libjsmn
    HG_REPOSITORY https://bitbucket.org/azimoff/jsmn
    HG_TAG 4b14865
    CMAKE_ARGS ${CMAKE_EXTRA_ARGS}
    SOURCE_DIR     "${CMAKE_BINARY_DIR}/libjsmn-src"
    BINARY_DIR     "${CMAKE_BINARY_DIR}/libjsmn-build"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in ${CMAKE_BINARY_DIR}/libjsmn-src/CMakeLists.txt
)

# target properties are not allowed for imported targets, so for now use a global variable
# for this until we find a way of setting the property on the libjsmn_static target
set (LIBJSMN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libjsmn-src ${CMAKE_BINARY_DIR}/libjsmn-build CACHE INTERNAL "")

add_library(libjsmn_static STATIC IMPORTED GLOBAL)
set_target_properties(libjsmn_static PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/libjsmn-build/libjsmn.a")

