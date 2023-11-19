cmake_minimum_required(VERSION 3.0.0)
project(dff_conv)

set(SOURCE_FILES main.cpp)

file(GLOB RWTOOLS "rwtools/*.cpp")

add_library(converter 
	lodepng/lodepng.cpp
    ${RWTOOLS}
	converter/Converter.cpp
	converter/ConverterGLTF.cpp
	${CMAKE_INSTALL_PREFIX}/externals/tinygltf/tiny_gltf.cc)

add_executable(dff_conv ${SOURCE_FILES})

target_link_libraries(dff_conv converter)

install(TARGETS dff_conv DESTINATION ${DFF_CONVERTER_INSTALL_BIN_DIR})
