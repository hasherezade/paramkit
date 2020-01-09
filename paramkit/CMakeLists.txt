cmake_minimum_required ( VERSION 2.8 )
project ( paramkit )

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

include_directories (
  include
)

set (srcs
	params.cpp
)

set (hdrs
	include/paramkit.h
	include/params.h
)

add_library ( ${PROJECT_NAME} STATIC ${hdrs} ${srcs} )
