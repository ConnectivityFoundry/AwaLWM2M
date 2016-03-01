cmake_minimum_required(VERSION 2.8.2)

project(libcoap NONE)

if (DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_EXTRA_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

Include (ExternalProject)
ExternalProject_Add(libcoap
    GIT_REPOSITORY https://github.com/obgm/libcoap
    GIT_TAG d48ab449fd05801e574e4966023589ed7dac500b
    CMAKE_ARGS ${CMAKE_EXTRA_ARGS}
    SOURCE_DIR     "${CMAKE_BINARY_DIR}/libcoap-src"
    BINARY_DIR     "${CMAKE_BINARY_DIR}/libcoap-build"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    PATCH_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/apply_patches ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

# for now use a global variable for this until we find a way of setting the property on the libcoap_static target
set (LIBCOAP_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libcoap-src ${CMAKE_BINARY_DIR}/libcoap-build CACHE INTERNAL "")

add_library(libcoap_static STATIC IMPORTED GLOBAL)
set_target_properties(libcoap_static PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/libcoap-build/libcoap.a")
#set_target_properties(libcoap_static PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${LIBCOAP_INCLUDE_DIR})


