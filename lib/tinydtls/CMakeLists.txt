cmake_minimum_required(VERSION 2.8.2)

project(tinydtls NONE)

if (DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_EXTRA_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

Include (ExternalProject)
ExternalProject_Add(tinydtls
    GIT_REPOSITORY https://git.eclipse.org/r/tinydtls/org.eclipse.tinydtls
    GIT_TAG 0016138fe3998552eee3987a1c09da43a23c9fb5
    CMAKE_ARGS ${CMAKE_EXTRA_ARGS}
    SOURCE_DIR     "${CMAKE_BINARY_DIR}/tinydtls-src"
    BINARY_DIR     "${CMAKE_BINARY_DIR}/tinydtls-build"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    PATCH_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/apply_patches ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

# for now use a global variable for this until we find a way of setting the property on the tinydtls_static target
set (TINYDTLS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/tinydtls-src ${CMAKE_BINARY_DIR}/tinydtls-build CACHE INTERNAL "")

add_library(tinydtls_static STATIC IMPORTED GLOBAL)
set_target_properties(tinydtls_static PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/tinydtls-build/libtinydtls.a")
#set_target_properties(tinydtls_static PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${TINYDTLS_INCLUDE_DIR})


