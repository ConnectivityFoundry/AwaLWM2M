set (API_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "API_TEST_DIR")

set (test_api_runner_SOURCES
  support/main/main.cc
  support/main/main_cmdline.c
  support/support.cc

  test_log.cc
  test_error.cc
  test_lwm2m_error.cc
  test_ipc.cc
  test_utils.cc
  test_iterator.cc
  test_path.cc
  test_string_iterator.cc
  test_path_iterator.cc
  test_client_iterator.cc
  test_path_result.cc
  test_array_types.cc
  test_objects_tree.cc
  test_queue.cc
  test_list.cc
  test_map.cc
  test_xmltree.cc
  test_operation_common.cc
  test_response_common.cc
  test_server_response.cc
  test_client.cc
  test_client_server_interaction.cc
  test_server.cc
  test_define_common.cc
  test_session_common.cc
  test_define_operation.cc
  test_value.cc
  
  test_client_session.cc
  test_client_define_operation.cc
  test_client_define_defaults.cc
  test_set_operation.cc
  test_get_operation.cc
  test_client_delete_operation.cc
  test_server_delete_operation.cc
  test_subscribe_operation.cc  
  test_observe_operation.cc  
  
  test_server_session.cc
  test_server_define_operation.cc
  test_list_clients_operation.cc
  test_write_operation.cc
  test_set_write_common.cc
  test_read_operation.cc
  test_write_attributes_operation.cc
  test_changeset.cc
  test_subscribe_observe_common.cc
  test_execute_operation.cc
)

set (test_api_runner_INCLUDE_DIRS
  ${GTEST_INCLUDE_DIR}
  ${API_INCLUDE_DIR}
  ${API_SRC_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client
  ${XML_INCLUDE_DIR}
)

set (test_api_runner_LIBRARIES
  gtest
  pthread
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

add_executable (test_api_runner ${test_api_runner_SOURCES})
target_include_directories (test_api_runner PRIVATE ${test_api_runner_INCLUDE_DIRS})
target_link_libraries (test_api_runner ${test_api_runner_LIBRARIES})

if (ENABLE_GCOV)
  target_link_libraries (test_api_runner gcov)
endif ()

# Testing
add_custom_command (
  OUTPUT test_api_runner_out.xml
  COMMAND test_api_runner --gtest_output=xml:test_api_runner_out.xml || true
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

if (RUN_TESTS)
  # always run test_api_runner
  add_custom_target (
    test_api_runner_TARGET ALL
    DEPENDS test_api_runner_out.xml
  )
endif ()


