cmake_minimum_required ( VERSION 3.0 )

project ( ParamKit )

# modules:
set ( M_PARAMKIT_LIB "paramkit" )
set ( M_PARAMKIT_DEMO "demo" )

# modules paths:
set ( PARAMKIT_DIR "${CMAKE_SOURCE_DIR}/${M_PARAMKIT_LIB}" CACHE PATH "ParamKit main path" )

# Add sub-directories
#
# libs
add_subdirectory ( paramkit )
set ( PARAMKIT_LIB $<TARGET_FILE:paramkit> CACHE PATH "ParamKit library path" )

#demos:
add_subdirectory ( demo )
add_dependencies ( demo paramkit )
