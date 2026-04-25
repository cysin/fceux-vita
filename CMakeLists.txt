cmake_minimum_required(VERSION 3.10)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

if (NOT PLATFORM_VITA)
    message(FATAL_ERROR "fceux-vita only supports PLATFORM_VITA=ON")
endif ()

set(OPTION_BUILTIN_LIBCONFIG ON CACHE BOOL "Build libconfig from sources")
set(OPTION_UPDATE_CONFIG OFF CACHE BOOL "Force config file updating")

# libcross2d handles Vita toolchain setup.
add_subdirectory(external/libcross2d)

project(fceux-vita)
set(PROJECT_AUTHOR cpasjuste)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

if (PLATFORM_VITA AND NOT TARGET_PLATFORM)
    set(TARGET_PLATFORM vita CACHE STRING "" FORCE)
endif ()

add_subdirectory(src/runtime)

##############
# FCEUX CORE
##############
set(FCEUX_DIR ${CMAKE_SOURCE_DIR}/external/fceux/src)
set(FCEUX_INC
        ${FCEUX_DIR}
        ${FCEUX_DIR}/drivers
)

# Core source files
set(FCEUX_SRC
        ${FCEUX_DIR}/asm.cpp
        ${FCEUX_DIR}/cart.cpp
        ${FCEUX_DIR}/cheat.cpp
        ${FCEUX_DIR}/conddebug.cpp
        ${FCEUX_DIR}/config.cpp
        ${FCEUX_DIR}/debug.cpp
        ${FCEUX_DIR}/debugsymboltable.cpp
        ${FCEUX_DIR}/drawing.cpp
        ${FCEUX_DIR}/emufile.cpp
        ${FCEUX_DIR}/fceu.cpp
        ${FCEUX_DIR}/fds.cpp
        ${FCEUX_DIR}/file.cpp
        ${FCEUX_DIR}/filter.cpp
        ${FCEUX_DIR}/ines.cpp
        ${FCEUX_DIR}/input.cpp
        ${FCEUX_DIR}/ld65dbg.cpp
        ${FCEUX_DIR}/movie.cpp
        ${FCEUX_DIR}/netplay.cpp
        ${FCEUX_DIR}/nsf.cpp
        ${FCEUX_DIR}/oldmovie.cpp
        ${FCEUX_DIR}/palette.cpp
        ${FCEUX_DIR}/profiler.cpp
        ${FCEUX_DIR}/ppu.cpp
        ${FCEUX_DIR}/sound.cpp
        ${FCEUX_DIR}/state.cpp
        ${FCEUX_DIR}/unif.cpp
        ${FCEUX_DIR}/video.cpp
        ${FCEUX_DIR}/vsuni.cpp
        ${FCEUX_DIR}/wave.cpp
        ${FCEUX_DIR}/x6502.cpp
        # Exclude: lua-engine.cpp (no Lua on Vita)
)

# All boards (NES mappers)
file(GLOB FCEUX_BOARDS_SRC ${FCEUX_DIR}/boards/*.cpp ${FCEUX_DIR}/boards/*.c)

# All input devices
file(GLOB FCEUX_INPUT_SRC ${FCEUX_DIR}/input/*.cpp)

# Utils (exclude backward.cpp - Linux stack tracer)
set(FCEUX_UTILS_SRC
        ${FCEUX_DIR}/utils/crc32.cpp
        ${FCEUX_DIR}/utils/endian.cpp
        ${FCEUX_DIR}/utils/general.cpp
        ${FCEUX_DIR}/utils/guid.cpp
        ${FCEUX_DIR}/utils/md5.cpp
        ${FCEUX_DIR}/utils/memory.cpp
        ${FCEUX_DIR}/utils/mutex.cpp
        ${FCEUX_DIR}/utils/timeStamp.cpp
        ${FCEUX_DIR}/utils/xstring.cpp
        ${FCEUX_DIR}/utils/ConvertUTF.c
        ${FCEUX_DIR}/utils/unzip.cpp
        ${FCEUX_DIR}/utils/ioapi.cpp
)

# Minimal drivers/common (only configSys needed by core)
set(FCEUX_COMMON_SRC
        ${FCEUX_DIR}/drivers/common/configSys.cpp
)

set(FCEUX_CFLAGS
        -DPSS_STYLE=1
        -DPUBLIC_RELEASE
        -DFRAMESKIP
        -DFCEUX_BUILD_TIMESTAMP="Vita"
        -D__FCEUX_VITA__
        -D__PEMU_VERSION_MAJOR__=${VERSION_MAJOR}
        -D__PEMU_VERSION_MINOR__=${VERSION_MINOR}
        -DHAVE_ASPRINTF
        -DPATH_MAX=4096
        -D__unix__
        -Wno-sign-compare
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-parentheses
        -Wno-unknown-pragmas
        -Wno-format-truncation
        -Wno-delete-non-virtual-dtor
        -Wno-reorder
        -Wno-write-strings
        -Wno-unused-local-typedefs
        -Wno-format-zero-length
        -Wno-narrowing
        -Wno-implicit-function-declaration
)

##############
# fceux-vita sources
##############
file(GLOB SRC_VITA
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.c
)
list(APPEND SRC_VITA ${CMAKE_SOURCE_DIR}/src/vita/port.c)

##############
# deps
##############
include(FindPkgConfig)
pkg_search_module(LIBARCHIVE REQUIRED libarchive)

set(CMAKE_CXX_STANDARD 17)

set(TITLE_ID "FCEU00001" CACHE STRING "" FORCE)

set(SRC
        ${SRC_VITA}
        ${FCEUX_SRC}
        ${FCEUX_BOARDS_SRC}
        ${FCEUX_INPUT_SRC}
        ${FCEUX_UTILS_SRC}
        ${FCEUX_COMMON_SRC}
)

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${FCEUX_INC}
)
target_compile_options(${PROJECT_NAME} PRIVATE ${FCEUX_CFLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        cross2dui cross2d
        ${LIBARCHIVE_LIBRARIES}
        z bz2 zstd
        ScePower_stub pthread
)

include(${CMAKE_SOURCE_DIR}/external/libcross2d/cmake/targets.cmake)

# add common skin data
add_custom_target(${PROJECT_NAME}.data.skin
        COMMAND ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/data/common/romfs -D DST=${CMAKE_CURRENT_BINARY_DIR}/data_romfs -P ${CMAKE_SOURCE_DIR}/external/libcross2d/cmake/copy_custom.cmake
        COMMAND ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/data/${TARGET_PLATFORM}/romfs -D DST=${CMAKE_CURRENT_BINARY_DIR}/data_romfs -P ${CMAKE_SOURCE_DIR}/external/libcross2d/cmake/copy_custom.cmake
)
add_dependencies(${PROJECT_NAME}.data.skin ${PROJECT_NAME}.data)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data.skin)
