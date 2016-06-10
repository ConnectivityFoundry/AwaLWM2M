set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g -std=c++11")
if (ENABLE_GCOV)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_executable (test_tools_runner
  ../../tools_common.c
  test_tools_common.cc
  ${API_TEST_DIR}/support/main/main.cc
  ${API_TEST_DIR}/support/main/main_cmdline.c
  ${API_TEST_DIR}/support/support.cc
  ${API_TEST_DIR}/support/process.cc
  ${API_TEST_DIR}/support/daemon.cc
  ${API_TEST_DIR}/support/log.cc
)

set_source_files_properties(${API_TEST_DIR}/support/main/main_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

target_include_directories (test_tools_runner PRIVATE ../..)
target_include_directories (test_tools_runner PRIVATE ${GTEST_INCLUDE_DIR})
target_include_directories (test_tools_runner PRIVATE ${API_INCLUDE_DIR})
target_include_directories (test_tools_runner PRIVATE ${API_TEST_DIR})
target_include_directories (test_tools_runner PRIVATE ${CORE_SRC_DIR})

target_link_libraries(test_tools_runner gtest)
target_link_libraries(test_tools_runner pthread)
target_link_libraries(test_tools_runner Awa_static)
if (ENABLE_GCOV)
  target_link_libraries(test_tools_runner gcov)
endif ()

# Testing
add_custom_command (
  OUTPUT test_tools_runner_out.xml
  COMMAND test_tools_runner --gtest_output=xml:test_tools_runner_out.xml || true
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

if (RUN_TESTS)
  # always run test_tools_runner
  add_custom_target (
    test_tools_runner_TARGET ALL
    DEPENDS test_tools_runner_out.xml
  )
endif ()
