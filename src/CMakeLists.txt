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

set_target_properties(dff_converter PROPERTIES PUBLIC_HEADER "${CMAKE_INSTALL_PREFIX}/externals/tinygltf/tiny_gltf.h;converter/Converter.h;converter/ConverterGLTF.h;rwtools/renderware.h")
set_target_properties(dff_converter_static PROPERTIES PUBLIC_HEADER "${CMAKE_INSTALL_PREFIX}/externals/tinygltf/tiny_gltf.h;converter/Converter.h;converter/ConverterGLTF.h;rwtools/renderware.h")


target_link_libraries(dff_conv dff_converter)
target_link_libraries(dff_conv dff_converter_static)

install(TARGETS dff_conv DESTINATION ${DFF_CONVERTER_RUNTIME_OUTPUT_DIRECTORY})
install(TARGETS dff_converter 
        DESTINATION ${DFF_CONVERTER_LIBRARY_OUTPUT_DIRECTORY}
        PUBLIC_HEADER DESTINATION ${DFF_CONVERTER_INCLUDE_OUTPUT_DIRECTORY}
)
install(TARGETS dff_converter_static 
        DESTINATION ${DFF_CONVERTER_LIBRARY_OUTPUT_DIRECTORY}
        PUBLIC_HEADER DESTINATION ${DFF_CONVERTER_INCLUDE_OUTPUT_DIRECTORY}
)