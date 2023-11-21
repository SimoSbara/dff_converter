cmake_minimum_required(VERSION 3.0.0)
project(dff_conv)

set(SOURCE_FILES main.cpp)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(GLOB RWTOOLS "rwtools/*.cpp")

add_library(dff_converter SHARED 
	lodepng/lodepng.cpp
    ${RWTOOLS}
	converter/Converter.cpp
	converter/ConverterGLTF.cpp
	${CMAKE_INSTALL_PREFIX}/externals/tinygltf/tiny_gltf.cc)

add_library(dff_converter_static STATIC 
	lodepng/lodepng.cpp
    ${RWTOOLS}
	converter/Converter.cpp
	converter/ConverterGLTF.cpp
	${CMAKE_INSTALL_PREFIX}/externals/tinygltf/tiny_gltf.cc)

add_executable(dff_conv ${SOURCE_FILES})

target_link_libraries(dff_conv dff_converter)
target_link_libraries(dff_conv dff_converter_static)

install(TARGETS dff_conv DESTINATION ${DFF_CONVERTER_INSTALL_BIN_DIR}/x${OSBit}/${CMAKE_BUILD_TYPE})
