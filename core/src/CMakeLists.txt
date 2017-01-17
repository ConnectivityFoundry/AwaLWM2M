set (CORE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "CORE_SRC_DIR")

set (CMAKE_C_FLAGS_DEBUG "-Wall -Werror -Wno-pointer-sign -g -DVERSION='\"${VERSION}\"' -D_GNU_SOURCE")
set (CMAKE_C_FLAGS_RELEASE "-Os -Wall -Werror -Wno-pointer-sign -DVERSION='\"${VERSION}\"' -D_GNU_SOURCE")
#SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> rcs <TARGET> <LINK_FLAGS> <OBJECTS>")

add_subdirectory (common)
add_subdirectory (bootstrap)
add_subdirectory (client)
add_subdirectory (server)
add_subdirectory (erbium)
