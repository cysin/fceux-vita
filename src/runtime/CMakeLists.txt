cmake_minimum_required(VERSION 3.10)

project(cross2dui)

include(FindPkgConfig)

set(CROSS2DUI_INCLUDE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
set(CROSS2DUI_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/load_error.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pemu_config.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/skin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui_emu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui_progressbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui_status_box.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui_video.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/skin/SkinnedRectangle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/skin/SkinnedText.cpp
)

set(FLAGS)

if (OPTION_UPDATE_CONFIG)
    list(APPEND FLAGS -D__UPDATE_CONFIG__)
endif ()

if (OPTION_FULLSCREEN)
    list(APPEND FLAGS -D__FULLSCREEN__)
endif ()

pkg_search_module(MINIZIP minizip)
if (MINIZIP_FOUND AND NOT PLATFORM_VITA)
    list(APPEND CROSS2DUI_INCLUDE ${MINIZIP_INCLUDE_DIRS})
    list(APPEND LDFLAGS ${MINIZIP_LIBRARIES})
else ()
    message(WARNING "minizip not found, using embedded one...")
    list(APPEND CROSS2DUI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/external)
    list(APPEND CROSS2DUI_SOURCE
            ${CMAKE_CURRENT_SOURCE_DIR}/external/minizip/ioapi.c
            ${CMAKE_CURRENT_SOURCE_DIR}/external/minizip/zip.c
            ${CMAKE_CURRENT_SOURCE_DIR}/external/minizip/unzip.c)
    list(APPEND FLAGS -DIOAPI_NO_64 -DUSE_FILE32API)
endif ()

if (PLATFORM_LINUX OR PLATFORM_WINDOWS)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    if (NOT PLATFORM_WINDOWS)
        list(APPEND LDFLAGS ${CMAKE_THREAD_LIBS_INIT} lzma dl)
    endif ()
elseif (PLATFORM_3DS)
endif ()

add_library(${PROJECT_NAME} STATIC ${CROSS2DUI_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC ${CROSS2DUI_INCLUDE})
target_compile_options(${PROJECT_NAME} PUBLIC ${FLAGS})
target_link_libraries(${PROJECT_NAME} cross2d ${LDFLAGS})
