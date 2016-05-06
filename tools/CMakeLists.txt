set (Tools
  awa-client-explore
  awa-client-get
  awa-client-set
  awa-client-delete
  awa-client-subscribe

  awa-server-explore
  awa-server-list-clients
  awa-server-read
  awa-server-write
  awa-server-delete
  awa-server-execute
  awa-server-write-attributes
  awa-server-observe
)

# Define tools are handled separately
set (DefineTools
  awa-server-define
  awa-client-define
)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror")

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# Virtual library to avoid building .o files twice:

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_HMAC_INCLUDE_DIR TARGET libhmac_static PROPERTY INCLUDE_DIRECTORIES)

add_library (Tools_object OBJECT tools_common.c changeset_common.c)
target_include_directories (Tools_object PUBLIC ${API_INCLUDE_DIR})
target_include_directories (Tools_object PRIVATE ${CORE_SRC_DIR})
target_include_directories (Tools_object PRIVATE ${LIB_B64_INCLUDE_DIR})
target_include_directories (Tools_object PRIVATE ${LIB_HMAC_INCLUDE_DIR})

foreach (tool ${DefineTools})
  # disable -Wall warnings for _cmdline.c files
  set_source_files_properties (${tool}_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)
  add_executable (${tool} ${tool}.c ${tool}_cmdline.c define_common.c $<TARGET_OBJECTS:Tools_object>)
  string (REGEX REPLACE "^.*-(.+)-.*$" "\\1" ToolType ${tool})
  string (TOUPPER ${ToolType} ToolType)
  target_compile_definitions (${tool} PRIVATE TOOL_${ToolType})
  target_include_directories (${tool} PRIVATE ${API_INCLUDE_DIR})
  target_include_directories (${tool} PRIVATE ${CORE_SRC_DIR})  
  target_link_libraries (${tool} Awa_static)
  target_link_libraries (${tool} libxml_static)
  target_link_libraries (${tool} libhmac_static)
  target_link_libraries (${tool} libb64_static)
  if (ENABLE_GCOV)
    target_link_libraries (${tool} gcov)
  endif ()
endforeach ()

install (TARGETS ${DefineTools}
  RUNTIME DESTINATION bin
)

foreach (tool ${Tools})

  # disable -Wall warnings for _cmdline.c files
  set_source_files_properties (${tool}_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)
  
  add_executable (${tool} ${tool}.c ${tool}_cmdline.c $<TARGET_OBJECTS:Tools_object>)
  target_include_directories (${tool} PRIVATE ${API_INCLUDE_DIR})
  target_include_directories (${tool} PRIVATE ${CORE_SRC_DIR})  
  target_link_libraries (${tool} Awa_static)
  target_link_libraries (${tool} libxml_static)
  if (ENABLE_GCOV)
    target_link_libraries (${tool} gcov)
  endif ()
endforeach (tool ${Tools})

install (TARGETS ${Tools}
  RUNTIME DESTINATION bin
)

if (BUILD_TESTS)
  add_subdirectory (tests/gtest)
endif ()
