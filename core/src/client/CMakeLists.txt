set (awa_clientd_SOURCES
  awa_clientd_cmdline.c
  lwm2m_registration.c
  lwm2m_bootstrap.c
  lwm2m_security_object.c
  lwm2m_acl_object.c
  lwm2m_server_object.c
  lwm2m_client_xml_handlers.c
  lwm2m_client_core.c
  lwm2m_static.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_interface.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_bootstrap_config.c
  ${CORE_SRC_DIR}/common/lwm2m_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_tlv.c
  ${CORE_SRC_DIR}/common/lwm2m_plaintext.c
  ${CORE_SRC_DIR}/common/lwm2m_prettyprint.c
  ${CORE_SRC_DIR}/common/lwm2m_opaque.c
  ${CORE_SRC_DIR}/common/lwm2m_tree_builder.c
  ${CORE_SRC_DIR}/common/lwm2m_observers.c
  ${CORE_SRC_DIR}/common/lwm2m_ipc.c
  lwm2m_object_tree.c

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

# static library libawa_static.a
add_library (awa_static STATIC  ${awa_clientd_SOURCES})
target_include_directories (awa_static PUBLIC ${awa_clientd_INCLUDE_DIRS})
set_property (TARGET awa_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries (awa_static awa_common_static libb64_static libhmac_static)

# libawa_static.so
add_library (awa_static_shared SHARED ${awa_clientd_SOURCES})
set_target_properties (awa_static_shared PROPERTIES OUTPUT_NAME "awa_static")
target_include_directories (awa_static_shared PUBLIC ${awa_clientd_INCLUDE_DIRS})
set_property (TARGET awa_static_shared PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries (awa_static_shared awa_common_static libb64_static libhmac_static)

# disable -Wall warnings for _cmdline.c files
set_source_files_properties (awa_clientd_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

add_executable (awa_clientd lwm2m_client.c)
target_include_directories (awa_clientd PRIVATE ${awa_clientd_INCLUDE_DIRS})
target_link_libraries (awa_clientd awa_static awa_common_static libb64_static libhmac_static)

if (ENABLE_GCOV)
  target_link_libraries (awa_clientd gcov)
endif ()

install (TARGETS awa_clientd
    RUNTIME DESTINATION bin
)

install (TARGETS awa_static_shared
    LIBRARY DESTINATION lib
)
