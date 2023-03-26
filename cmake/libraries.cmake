find_package(ImageMagick REQUIRED COMPONENTS Magick++ MagickCore)
find_package(Freetype REQUIRED)
find_package(LibXml2 REQUIRED)

find_package(Boost 1.71.0 REQUIRED COMPONENTS locale)
find_package(ICU 66.1 REQUIRED COMPONENTS in uc dt)

include(cmake/magick.cmake)

find_library(YAML yaml-cpp REQUIRED)
