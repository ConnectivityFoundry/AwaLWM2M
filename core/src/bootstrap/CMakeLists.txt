set (awa_bootstrapd_SOURCES
  awa_bootstrapd_cmdline.c
  lwm2m_bootstrap_server.c
  lwm2m_bootstrap.c
  lwm2m_bootstrap_core.c
  ${CORE_SRC_DIR}/server/lwm2m_object_defs.c
  ${CORE_SRC_DIR}/common/lwm2m_bootstrap_config.c
  ${CORE_SRC_DIR}/common/lwm2m_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_tlv.c
  ${CORE_SRC_DIR}/common/lwm2m_plaintext.c
  ${CORE_SRC_DIR}/common/lwm2m_prettyprint.c
  ${CORE_SRC_DIR}/common/lwm2m_opaque.c
  ${CORE_SRC_DIR}/common/lwm2m_tree_builder.c
)

set (awa_bootstrapd_INCLUDE_DIRS
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/bootstrap
)

if (WITH_JSON)
  list (APPEND awa_bootstrapd_SOURCES
    ${CORE_SRC_DIR}/common/lwm2m_json.c
  )
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_bootstrapd_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR}
  )
endif ()

add_definitions (-DLWM2M_BOOTSTRAP)

if (ENABLE_GCOV)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# disable -Wall warnings for _cmdline.c files
set_source_files_properties (awa_bootstrapd_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

add_executable (awa_bootstrapd ${awa_bootstrapd_SOURCES})
target_include_directories (awa_bootstrapd PRIVATE ${awa_bootstrapd_INCLUDE_DIRS})
target_link_libraries (awa_bootstrapd awa_common_static)

if (ENABLE_GCOV)
  target_link_libraries (awa_bootstrapd gcov)
endif ()

install (TARGETS awa_bootstrapd
    RUNTIME DESTINATION bin
)

