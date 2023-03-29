cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ttf-2-bpp)

option(TTF_USE_BOXER "Use the boxer library for notification windows." Off)

set(TTF2BPP_RENDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/render")
set(TTF2BPP_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/configuration")
set(TTF2BPP_UNICODE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/unicode")
set(TTF2BPP_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

include(cmake/libraries.cmake)
include(cmake/unicode.cmake)

file(GLOB_RECURSE TTF_LIB_FILES
    ${TTF2BPP_RENDER_PATH}/renderer.cpp
    ${TTF2BPP_RENDER_PATH}/pnghelper.h
    ${TTF2BPP_RENDER_PATH}/pnghelper.cpp
    ${TTF2BPP_RENDER_PATH}/render_glyph_conv.cpp
    ${TTF2BPP_RENDER_PATH}/render_glyph_conv.h
    ${TTF2BPP_CONFIG_PATH}/glyphs.cpp
    ${TTF2BPP_CONFIG_PATH}/configuration.cpp
    ${TTF2BPP_CONFIG_PATH}/convert.cpp
    ${TTF2BPP_CONFIG_PATH}/config_impl.h
    ${TTF2BPP_INCLUDE_PATH}/glyphs.h
    ${TTF2BPP_INCLUDE_PATH}/renderer.h
    ${TTF2BPP_INCLUDE_PATH}/configuration.h
    ${TTF2BPP_INCLUDE_PATH}/ttf2bpp.h
    ${TTF2BPP_INCLUDE_PATH}/ttf2bpp_exports.h
)

add_library(ttf2bpp-lib ${TTF_LIB_FILES})

target_link_libraries(ttf2bpp-lib ${FREETYPE_TARGET} ${expat_LIBRARIES} Boost::locale ${ICU_LIBRARIES} ttf2bpp-unicode)

target_compile_definitions(ttf2bpp-lib PUBLIC TTF_BPP_DLL=1)

target_include_directories(ttf2bpp-lib PUBLIC
    ${expat_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${TTF2BPP_INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
    ${YAML_INCLUDE_DIR}
)

check_pngpp(ttf2bpp-lib)

add_executable(ttf-2-bpp
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/noitifer/notifier.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/noitifer/notifier.cpp"
)


find_cxxopts_includes(ttf-2-bpp)

ttf_check_boxer(ttf-2-bpp)

target_link_libraries(ttf-2-bpp ttf2bpp-lib ${YAML} ${boxer_LIBRARIES})

target_include_directories(ttf-2-bpp PUBLIC ${TTF2BPP_INCLUDE_PATH})

setup_unicode_db()

set(CMAKE_VERBOSE_MAKEFILE TRUE)
