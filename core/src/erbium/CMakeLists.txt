set (awa_erbium_SOURCES
  er-coap.c
  er-coap-engine.c
  er-coap-transactions.c
#  er-coap-observe.c
  er-coap-separate.c
#  er-coap-res-well-known-core.c
  er-coap-block1.c
#  er-coap-observe-client.c
)

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
#get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_HMAC_INCLUDE_DIR TARGET libhmac_static PROPERTY INCLUDE_DIRECTORIES)

# currently the API and core are a little tangled so we need to define the include
# path here. the API tests depend on core, which depends on the API.
set (API_INCLUDE_DIR ../../../api/include CACHE INTERNAL "API_INCLUDE_DIR")

set (awa_erbium_INCLUDE_DIRS
  ${API_INCLUDE_DIR}
  ${LIB_B64_INCLUDE_DIR}
  ${LIB_HMAC_INCLUDE_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client
  ${CORE_SRC_DIR}/erbium
  #${CORE_SRC_DIR}/../../api/src
  #${CORE_SRC_DIR}/../../api/include
)

# TODO - remove
set (awa_erbium_LIBS
  libb64_static
  libhmac_static
)

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# From Contiki Makefiles
add_definitions (-DREST_MAX_CHUNK_SIZE=512)
add_definitions (-DPOSIX)
remove_definitions (-DWITH_JSON)
#add_definitions (-DUIP_CONF_BUFFER_SIZE=4096)

# static library liberbiumstatic.a
add_library (awa_erbiumstatic STATIC ${awa_erbium_SOURCES})
set_target_properties (awa_erbiumstatic PROPERTIES OUTPUT_NAME "erbiumstatic")
set_target_properties (awa_erbiumstatic PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(awa_erbiumstatic PUBLIC ${awa_erbium_INCLUDE_DIRS})
target_link_libraries (awa_erbiumstatic ${awa_erbium_LIBS})

# TODO - needed? c.f. libawa_static)
#if (ENABLE_GCOV)
#  target_link_libraries (awa_erbiumstatic gcov)
#endif ()

# liberbiumstatic.so
add_library (awa_erbiumstatic_shared SHARED ${awa_erbium_SOURCES})
set_target_properties (awa_erbiumstatic_shared PROPERTIES OUTPUT_NAME "erbiumstatic")
set_target_properties (awa_erbiumstatic_shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories (awa_erbiumstatic_shared PUBLIC ${awa_erbium_INCLUDE_DIRS})
target_link_libraries (awa_erbiumstatic_shared ${awa_erbium_LIBS})

install (TARGETS awa_erbiumstatic_shared
    LIBRARY DESTINATION lib
)
