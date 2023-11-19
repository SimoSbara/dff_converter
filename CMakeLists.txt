cmake_minimum_required(VERSION 3.0.0)
project(dff_converter VERSION 0.1.0 LANGUAGES C CXX)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif(MSVC)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(DFF_CONVERTER_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(DFF_CONVERTER_HEADERS_DIR 
	${PROJECT_SOURCE_DIR}/src/converter
	${PROJECT_SOURCE_DIR}/src/lodepng
	${PROJECT_SOURCE_DIR}/src/rwtools
	${PROJECT_SOURCE_DIR}/externals/tinygltf
)

message(DFF_CONVERTER_HEADERS_DIR="${DFF_CONVERTER_HEADERS_DIR}")

include_directories(${DFF_CONVERTER_INSTALL_INCLUDE_DIR})
include_directories(${DFF_CONVERTER_HEADERS_DIR})

add_subdirectory(src)

