set (awa_serverd_SOURCES
  lwm2m_server.c
  awa_serverd_cmdline.c
  lwm2m_server_xml_handlers.c
  lwm2m_server_xml_events.c
  lwm2m_server_xml_registered_entity_tree.c
  
  ${DAEMON_SRC_DIR}/common/lwm2m_xml_interface.c
  ${DAEMON_SRC_DIR}/common/lwm2m_xml_serdes.c
  ${DAEMON_SRC_DIR}/common/lwm2m_ipc.c
  ${DAEMON_SRC_DIR}/common/lwm2m_events.c
  ${DAEMON_SRC_DIR}/common/ipc_session.c
  ${DAEMON_SRC_DIR}/common/xml.c
  ${DAEMON_SRC_DIR}/common/objdefs.c
  
    ######################## TODO REMOVE ########################
  # TODO: extract components common to both Core and API
  # FIXME: API_SRC_DIR is not in the cmake cache at the time this is read
  #${API_SRC_DIR}/path.c
  #${API_SRC_DIR}/objects_tree.c
  #${API_SRC_DIR}/log.c
  #${API_SRC_DIR}/error.c
  #${API_SRC_DIR}/utils.c
  ${CORE_SRC_DIR}/../../api/src/path.c
  ${CORE_SRC_DIR}/../../api/src/objects_tree.c
  ${CORE_SRC_DIR}/../../api/src/log.c
  ${CORE_SRC_DIR}/../../api/src/error.c
  ${CORE_SRC_DIR}/../../api/src/lwm2m_error.c
  ${CORE_SRC_DIR}/../../api/src/utils.c
  ${CORE_SRC_DIR}/../../api/src/write_mode.c
  #############################################################
)

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)

set (awa_serverd_INCLUDE_DIRS
  ${LIB_XML_INCLUDE_DIR}
  ${LIB_B64_INCLUDE_DIR}

  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/server

  ${DAEMON_SRC_DIR}/common

  ######################## TODO REMOVE ########################
  # TODO: extract components common to both Core and API
  # FIXME: API_INCLUDE_DIR is not in the cmake cache at the time this is read
  #${API_INCLUDE_DIR}
  #${API_SRC_DIR}
  ${CORE_SRC_DIR}/../../api/src
  ${CORE_SRC_DIR}/../../api/include
  #############################################################
)

if (WITH_JSON)
  list (APPEND awa_serverd_SOURCES
    ${CORE_SRC_DIR}/common/lwm2m_json.c
  )
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_serverd_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR})
endif ()

add_definitions (-DLWM2M_SERVER)

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# disable -Wall warnings for _cmdline.c files
set_source_files_properties (awa_serverd_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

add_executable (awa_serverd ${awa_serverd_SOURCES})
target_include_directories (awa_serverd PRIVATE ${awa_serverd_INCLUDE_DIRS})
target_link_libraries (awa_serverd awa_server_static awa_common_static libb64_static libxml_static)

if (ENABLE_GCOV)
   target_link_libraries (awa_serverd gcov)
endif ()

install (TARGETS awa_serverd
  RUNTIME DESTINATION bin
)
