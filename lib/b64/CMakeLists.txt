add_library (libb64_static b64.c)
set_property (TARGET libb64_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties (libb64_static PROPERTIES OUTPUT_NAME "b64")
target_include_directories (libb64_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
