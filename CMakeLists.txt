cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ttf-test)


include(cmake/libraries.cmake)

set(TTF2BPP_RENDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/render")
set(TTF2BPP_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/configuration")
set(TTF2BPP_UNICODE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/unicode")
set(TTF2BPP_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE TTF_LIB_FILES
    ${TTF2BPP_RENDER_PATH}/renderer.cpp
    ${TTF2BPP_RENDER_PATH}/render_glyph_conv.cpp
    ${TTF2BPP_RENDER_PATH}/render_glyph_conv.h
    ${TTF2BPP_CONFIG_PATH}/configuration.cpp
    ${TTF2BPP_CONFIG_PATH}/config_impl.h
    ${TTF2BPP_INCLUDE_PATH}/renderer.h
    ${TTF2BPP_UNICODE_PATH}/unicodedatabase.cpp
    ${TTF2BPP_INCLUDE_PATH}/unicodedatabase.h
    ${TTF2BPP_INCLUDE_PATH}/configuration.h
    ${TTF2BPP_INCLUDE_PATH}/ttf2bpp.h
)

add_library(ttf2bpp-lib ${TTF_LIB_FILES})

target_link_libraries(ttf2bpp-lib Freetype::Freetype ${ImageMagick_LIBRARIES} LibXml2::LibXml2 Boost::locale ${ICU_LIBRARIES})

target_include_directories(ttf2bpp-lib PUBLIC
    ${FREETYPE_INCLUDE_DIRS}
    ${ImageMagick_INCLUDE_DIRS}
    ${TTF2BPP_INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
)

check_magick(ttf2bpp-lib)

add_executable(ttf-test "${CMAKE_CURRENT_SOURCE_DIR}/interface/main.cpp")

target_link_libraries(ttf-test ttf2bpp-lib ${YAML})

target_include_directories(ttf-test PUBLIC ${TTF2BPP_INCLUDE_PATH})

set(UNICODE_DB_FILE ${CMAKE_CURRENT_BINARY_DIR}/ucd.all.grouped.xml)

file(GLOB UNICODE_FILE_EXISTS ${UNICODE_DB_FILE})

if (NOT UNICODE_FILE_EXISTS)
    message(STATUS "Downloading Unicode database file.")
    set (UNICODE_ZIP_FILE ${CMAKE_CURRENT_BINARY_DIR}/ucd.all.grouped.zip)

    include(FetchContent)

    file(DOWNLOAD https://www.unicode.org/Public/UCD/latest/ucdxml/ucd.all.grouped.zip ${UNICODE_ZIP_FILE})
    file(ARCHIVE_EXTRACT INPUT ${UNICODE_ZIP_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(REMOVE ${UNICODE_ZIP_FILE})
endif()
