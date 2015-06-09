add_library (libxml_static xmltree.c xmlparser.c)
set_property (TARGET libxml_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties (libxml_static PROPERTIES OUTPUT_NAME "xml")
target_include_directories (libxml_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
