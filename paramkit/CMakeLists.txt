cmake_minimum_required ( VERSION 2.8 )
project ( paramkit )

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

include_directories (
  include
)

set (srcs
	pk_util.cpp
	strings_util.cpp
)

set (hdrs
	include/paramkit.h
	include/param.h
	include/params.h
	include/term_colors.h
	include/pk_util.h
	include/strings_util.h
)

add_library ( ${PROJECT_NAME} STATIC ${hdrs} ${srcs} )
