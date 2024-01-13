
find_package(Freetype REQUIRED)
if (Freetype::Freetype)
    set(FREETYPE_TARGET Freetype::Freetype)
else()
    set(FREETYPE_TARGET freetype)
endif()

find_package(ICU 66.1 REQUIRED COMPONENTS in uc dt)

find_library(YAML yaml-cpp)
if(NOT YAML)
    find_package(yaml-cpp)
    set(YAML ${yaml-cpp_LIBRARIES})
    set(YAML_INCLUDE_DIR ${yaml-cpp_INCLUDE_DIRS})
endif()


function(find_cxxopts_includes Target)
    find_package(cxxopts)

    if (cxxopts_INCLUDE_DIRS)
        target_include_directories(${Target} PUBLIC ${cxxopts_INCLUDE_DIRS})
    endif()
endfunction(find_cxxopts_includes)

function(check_pngpp Target)
    find_package(pngpp QUIET)
    if (pngpp_INCLUDE_DIRS)
        find_package(PNG REQUIRED)

        target_include_directories(${Target} PUBLIC ${pngpp_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
        target_link_libraries(${Target} ${PNG_LIBRARIES})
    else()
        message(STATUS "png++ not provided by package manager.")
        find_package(PNG REQUIRED)
        target_include_directories(${Target} PUBLIC ${PNG_INCLUDE_DIRS})
        target_link_libraries(${Target} ${PNG_LIBRARIES})
    endif()
endfunction(check_pngpp)

if (expat_DIR)
    message(STATUS "Checking provided expat.")
    find_package(expat REQUIRED)
else()
    # hack required because of conan: https://github.com/conan-io/conan/issues/10387
    set(PKG_CONF_PREF ${CMAKE_FIND_PACKAGE_PREFER_CONFIG})
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG Off)
    message(STATUS "Checking System expat.")
    find_package(EXPAT REQUIRED)
    message(STATUS ${EXPAT_LIBRARIES})
    set(expat_LIBRARIES EXPAT::EXPAT)

    set(expat_INCLUDE_DIRS ${EXPAT_INCLUDE_DIRS})
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ${PKG_CONF_PREF})
endif()

function(ttf_check_boxer Target)
    if (TTF_USE_BOXER)
        find_library(BOXER Boxer)
        find_package(boxer REQUIRED)
        target_include_directories(${Target} PRIVATE ${boxer_INCLUDE_DIRS})
        target_compile_definitions(${Target} PUBLIC TTF_USE_BOXER)
        set(boxer_LIBRARIES ${BOXER})
        if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
            list(APPEND boxer_LIBRARIES ${GTK3_LIBRARIES})
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            list(APPEND boxer_LIBRARIES user32.lib)
        endif()
        set(boxer_LIBRARIES ${boxer_LIBRARIES} PARENT_SCOPE)
    endif()
endfunction()
