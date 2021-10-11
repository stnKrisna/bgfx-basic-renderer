cmake_minimum_required(VERSION 3.8)

# Project config
project(bgfx_tutorial)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/extras/cmake" ${CMAKE_MODULE_PATH})
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist${DIST_SUFFIX}") # DIST_SUFFIX must be set from the command line. Useful when we want to build for 32-bit & 64-bit

# Versioning
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_BUILD ${unix_time})

# Platform specific compiler
# hosts
if(CMAKE_HOST_APPLE)
    set(PRJ_HOST_MACOS            1)
    set(PRJ_SHADERS_COMPILER    "${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bgfx/bin/shaderc_osx")
    set(PRJ_TEXTURE_COMPILER    "${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bgfx/bin/texturec_osx")
    set(PRJ_BIN2C               "${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bin2c/bin2c")
    set(PRJ_PYTHON              "/usr/bin/python")

elseif(CMAKE_HOST_WIN32)
    set(PRJ_HOST_WINDOWS        1)
    set(PRJ_SHADERS_COMPILER    "${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bgfx/bin/shaderc_win.exe")
    set(PRJ_TEXTURE_COMPILER    "${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bgfx/bin/texturec_win.exe")
    set(PRJ_BIN2C               "${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bin2c/bin2c.exe")
    if(PRJ_TARGET_EMSCRIPTEN)
        # Emscripten overrides python but we still want to use system python because of installed packages
        set(PRJ_PYTHON            py -3)
    else()
        set(PRJ_PYTHON            "python")
    endif()

else()
    message(FATAL_ERROR "unknown host")
endif()

# built-in shaders and textures
# SEE HERE: https://github.com/beardsvibe/leengine/blob/master/CMakeLists.txt
if(EMSCRIPTEN)
    set(PRJ_TARGET_EMSCRIPTEN        1)
    set(PRJ_SHADERS_PLATFORM        "asm.js")
    set(PRJ_BUILD_EXECUTABLE        1)
    #set(PRJ_FMOD_AVAILABLE            1)

    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bgfx/libs/emscripten
    )

elseif(ANDROID_PLATFORM)
    set(PRJ_TARGET_ANDROID            1)
    set(PRJ_SHADERS_PLATFORM        "android")
    set(PRJ_BUILD_SHARED_LIB        1)
    #set(PRJ_FMOD_AVAILABLE            1)

    # TODO fix android CPU arch build
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/bgfx/libs/android_armv7a
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/tools/fmod/libs/android/armeabi-v7a
    )

elseif(APPLE AND IOS)
    set(PRJ_TARGET_IOS                1)
    set(PRJ_SHADERS_PLATFORM        "ios")

elseif(APPLE)
    set(PRJ_TARGET_MACOS             1)
    set(PRJ_SHADERS_PLATFORM         "osx")
    set(PRJ_SHADERS_ARGS_VS            -p metal)
    set(PRJ_SHADERS_ARGS_FS            -p metal)
    set(PRJ_BUILD_EXECUTABLE        1)
    #set(PRJ_FMOD_AVAILABLE            1)

    if(NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        message(FATAL_ERROR "only 64 bit builds are supported")
    endif()

elseif(WIN32)
    set(PRJ_TARGET_WINDOWS            1)
    set(PRJ_SHADERS_PLATFORM        "windows")
    set(PRJ_SHADERS_ARGS_VS            -p vs_4_0 -O 3)
    set(PRJ_SHADERS_ARGS_FS            -p ps_4_0 -O 3)
    set(PRJ_BUILD_EXECUTABLE        1)
    #set(PRJ_FMOD_AVAILABLE            1)

    if(NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        message(FATAL_ERROR "only 64 bit builds are supported")
    endif()

    # hotfix bgfx debug build for non release builds on Windows, because of STL
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        set(BGFX_DEBUG 1)
    endif()
else()
    message(FATAL_ERROR "unknown target")
endif()

set(gen_src)
macro(compileShaders SRCS)
    foreach(FILE ${SRCS})
        get_filename_component(ARRAY_NAME "${FILE}" NAME_WE)
        
        if    ("${FILE}" MATCHES ".*\\.sc")
            continue()
        endif()
        
        if    ("${FILE}" MATCHES ".*\\.fs")
           set(SHADER_TYPE "fragment")
           set(SHADER_ARGS ${PRJ_SHADERS_ARGS_FS})
           set(ARRAY_NAME "${ARRAY_NAME}_fs")
        elseif("${FILE}" MATCHES ".*\\.vs")
            set(SHADER_TYPE "vertex")
            set(SHADER_ARGS ${PRJ_SHADERS_ARGS_VS})
            set(ARRAY_NAME "${ARRAY_NAME}_vs")
        elseif("${FILE}" MATCHES ".*\\.cs")
            set(SHADER_TYPE "compute")
            set(ARRAY_NAME "${ARRAY_NAME}_cs")
        endif()
        
        list(APPEND gen_src ${FILE}.h)
        add_custom_command(
            OUTPUT ${FILE}.h
            COMMAND ${PRJ_SHADERS_COMPILER}
            -i extras/tools/bgfx/include --type ${SHADER_TYPE} --platform ${PRJ_SHADERS_PLATFORM} ${SHADER_ARGS}
            -f ${FILE} -o ${FILE}.h --bin2c ${ARRAY_NAME}
            DEPENDS ${FILE} ${PRJ_SHADERS_COMPILER}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endmacro()

list(APPEND gen_src ${CMAKE_CURRENT_SOURCE_DIR}/src/_missing_texture.h)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/_missing_texture.h
    COMMAND ${PRJ_TEXTURE_COMPILER} -f src/_missing_texture.png -o src/_missing_texture.ktx -t RGBA8
    COMMAND ${PRJ_BIN2C} -n _missing_texture -o src/_missing_texture.h src/_missing_texture.ktx
    DEPENDS src/_missing_texture.png ${PRJ_TEXTURE_COMPILER} ${PRJ_BIN2C}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Find & link SDL2
find_package(SDL2 REQUIRED)

# Include internal engine cmake file
include(bgfxCMakeLists.txt)

file(GLOB ALL_SHADERS
    ${PROJECT_SOURCE_DIR}/src/*.vs
    ${PROJECT_SOURCE_DIR}/src/*.fs
    ${PROJECT_SOURCE_DIR}/src/*.cs
    ${PROJECT_SOURCE_DIR}/src/*.sc

    ${PROJECT_SOURCE_DIR}/src/*/*.vs
    ${PROJECT_SOURCE_DIR}/src/*/*.fs
    ${PROJECT_SOURCE_DIR}/src/*/*.cs
    ${PROJECT_SOURCE_DIR}/src/*/*.sc

    ${PROJECT_SOURCE_DIR}/src/*/*/*.vs
    ${PROJECT_SOURCE_DIR}/src/*/*/*.fs
    ${PROJECT_SOURCE_DIR}/src/*/*/*.cs
    ${PROJECT_SOURCE_DIR}/src/*/*/*.sc
)
compileShaders("${ALL_SHADERS}")

file(GLOB ALL_SOURCE
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.c

    ${PROJECT_SOURCE_DIR}/src/*/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*/*.c

    ${PROJECT_SOURCE_DIR}/src/*/*/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*/*/*.c
    
    ${PROJECT_SOURCE_DIR}/src/*/*/*/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*/*/*/*.c
)

list(APPEND ALL_SOURCE ${gen_src})

file(GLOB ALL_HEADER
    ${PROJECT_SOURCE_DIR}/src/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*.h

    ${PROJECT_SOURCE_DIR}/src/*/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*/*.h

    ${PROJECT_SOURCE_DIR}/src/*/*/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*/*/*.h
    
    ${PROJECT_SOURCE_DIR}/src/*/*/*/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*/*/*/*.h
)

# Process file groups
message("Grouping source files...")
macro(addGroup SRCS)
    foreach(FILE ${SRCS})
        # Get the directory of the source file
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

        # Remove common directory prefix to make the group
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

        # Make sure we are using windows slashes
        string(REPLACE "/" "\\" GROUP "${GROUP}")

        # Group into "Source Files" and "Header Files"
        if    ("${FILE}" MATCHES ".*\\.cpp")
           set(GROUP "Source Files${GROUP}")
           
        elseif("${FILE}" MATCHES ".*\\.mm")
            set(GROUP "Source Files${GROUP}")
            
        elseif("${FILE}" MATCHES ".*\\.c")
            set(GROUP "Source Files${GROUP}")
        
        elseif("${FILE}" MATCHES ".*\\.sc")
            set(GROUP "Source Files${GROUP}")
            
        elseif("${FILE}" MATCHES ".*\\.h")
           set(GROUP "Header Files${GROUP}")
           
        elseif("${FILE}" MATCHES ".*\\.hpp")
           set(GROUP "Header Files${GROUP}")
           
        elseif("${FILE}" MATCHES ".*\\.vs")
           set(GROUP "Shader Files${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.fs")
           set(GROUP "Shader Files${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.cs")
           set(GROUP "Shader Files${GROUP}")
        endif()

        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endmacro()

addGroup("${ALL_HEADER}")
addGroup("${ALL_SOURCE}")
addGroup("${ALL_SHADERS}")

add_executable(${CMAKE_PROJECT_NAME} ${ALL_HEADER} ${ALL_SOURCE} ${ALL_SHADERS})

# Include libraries
include_directories(
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${SDL2_LIBRARY}
    bgfx bx bimg
)

# Compiler flags
target_compile_features(${CMAKE_PROJECT_NAME}
    PRIVATE
    cxx_std_17
)

if(APPLE)
    # Also compile objective-c on apple machines
    set(CMAKE_CXX_FLAGS "-x objective-c++")
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE
    -Wall -Wextra -Wpedantic -fms-extensions
)
