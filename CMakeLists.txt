cmake_minimum_required(VERSION 3.20)

project(ttf-test)

find_package(ImageMagick REQUIRED COMPONENTS Magick++ MagickCore)
find_package(Freetype REQUIRED)

add_executable(ttf-test main.cpp)

set_property(TARGET ttf-test PROPERTY CXX_STANDARD 20)

target_link_libraries(ttf-test Freetype::Freetype ${ImageMagick_LIBRARIES})

target_include_directories(ttf-test PUBLIC ${FREETYPE_INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS})

include(cmake/magick.cmake)

check_magick(ttf-test)
