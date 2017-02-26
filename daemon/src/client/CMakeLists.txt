set (awa_clientd_SOURCES
  lwm2m_client.c
  awa_clientd_cmdline.c
  lwm2m_client_xml_handlers.c
  ${DAEMON_SRC_DIR}/common/lwm2m_xml_interface.c
  ${DAEMON_SRC_DIR}/common/lwm2m_xml_serdes.c
  ${DAEMON_SRC_DIR}/common/lwm2m_ipc.c
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
  #############################################################
)

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_HMAC_INCLUDE_DIR TARGET libhmac_static PROPERTY INCLUDE_DIRECTORIES)

set (awa_clientd_INCLUDE_DIRS
  ${LIB_XML_INCLUDE_DIR}
  ${LIB_B64_INCLUDE_DIR}
  ${LIB_HMAC_INCLUDE_DIR}

  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client
  
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
  list (APPEND awa_clientd_SOURCES
    ${CORE_SRC_DIR}/common/lwm2m_json.c
  )
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_clientd_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR}
  )
endif ()

add_definitions (-DLWM2M_CLIENT)

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# disable -Wall warnings for _cmdline.c files
set_source_files_properties (awa_clientd_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

add_executable (awa_clientd ${awa_clientd_SOURCES})
target_include_directories (awa_clientd PRIVATE ${awa_clientd_INCLUDE_DIRS})
target_link_libraries (awa_clientd awa_static awa_common_static libb64_static libhmac_static libb64_static libxml_static)

if (ENABLE_GCOV)
  target_link_libraries (awa_clientd gcov)
endif ()

install (TARGETS awa_clientd
    RUNTIME DESTINATION bin
)
