set (STATIC_API_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "STATIC_API_TEST_DIR")

set (test_static_api_runner_SOURCES
  ../tests/support/main/main.cc
  ../tests/support/main/main_cmdline.c
  ../tests/support/support.cc
  ../tests/support/process.cc
  ../tests/support/daemon.cc

  support/static_api_support.cc
  test_static_api.cc
  test_static_api_with_pointer.cc
  test_static_api_with_pointer_deprecated.cc
  test_static_api_handlers.cc
  test_static_api_handlers_deprecated.cc
  test_static_api_get_resource_instance_pointer.cc
  test_static_api_get_resource_instance_pointer_deprecated.cc
  test_static_api_create_delete.cc
  test_static_api_create_delete_deprecated.cc
)

set (test_static_api_runner_INCLUDE_DIRS
  ${GTEST_INCLUDE_DIR}
  ${API_INCLUDE_DIR}
  ${API_SRC_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${XML_INCLUDE_DIR}
)

set (test_static_api_runner_LIBRARIES
  gtest
  pthread
  awa_static
  awa_common_static
  Awa_static
  libb64_static
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g -std=c++11")
if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# Disable format-extra-args for this source file:
set_source_files_properties (test_utils.cc PROPERTIES COMPILE_FLAGS -Wno-format-extra-args)

set_source_files_properties (main/main_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

add_executable (test_static_api_runner ${test_static_api_runner_SOURCES})
target_include_directories (test_static_api_runner PRIVATE ${test_static_api_runner_INCLUDE_DIRS})
target_link_libraries (test_static_api_runner ${test_static_api_runner_LIBRARIES})

if (ENABLE_GCOV)
  target_link_libraries (test_static_api_runner gcov)
endif ()

# Testing
add_custom_command (
  OUTPUT test_static_api_runner_out.xml
  COMMAND test_static_api_runner --gtest_output=xml:test_static_api_runner_out.xml || true
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

if (RUN_TESTS)
  # always run test_static_api_runner
  add_custom_target (
    test_static_api_runner_TARGET ALL
    DEPENDS test_static_api_runner_out.xml
  )
endif ()


