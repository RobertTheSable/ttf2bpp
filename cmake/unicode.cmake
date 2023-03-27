function(setup_unicode_db)
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
endfunction()

file(GLOB_RECURSE TTF_UNICODE_FILES
    "${TTF2BPP_UNICODE_PATH}/unicodedatabase.cpp" "${TTF2BPP_INCLUDE_PATH}/unicodedatabase.h"
)

# unicode components need to be separate to avoid conflicts with imagemagick, or something :V
add_library(ttf2bpp-unicode SHARED ${TTF_UNICODE_FILES})

target_include_directories(ttf2bpp-unicode PUBLIC ${TTF2BPP_INCLUDE_PATH})

target_link_libraries(ttf2bpp-unicode PRIVATE ${expat_LIBRARIES})

target_compile_definitions(ttf2bpp-unicode PUBLIC TTF_UNICODE_DLL=1)
