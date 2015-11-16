CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(MINGW_BUILD)
  SET(CMAKE_SYSTEM_NAME Generic)
ENDIF()

INCLUDE(CheckIncludeFile)

PROJECT(mehstation)

SET(
        MEHSTATION_SOURCES
        src/main.c
        src/system/app.c
        src/system/db.c
        src/system/flags.c
        src/system/input.c
        src/system/message.c
        src/system/os_linux.c
        src/system/os_windows.c
        src/system/settings.c
        src/system/transition.c
        src/system/db/executable.c
        src/system/db/executable_resource.c
        src/system/db/mapping.c
        src/system/db/platform.c
        src/view/image.c
        src/view/screen.c
        src/view/text.c
        src/view/video.c
        src/view/window.c
        src/view/widget_image.c
        src/view/widget_rect.c
        src/view/widget_text.c
        src/view/widget_video.c
        src/view/screen/executable_list.c
        src/view/screen/exec_list_video.c
        src/view/screen/fade.c
        src/view/screen/exec_desc.c
        src/view/screen/exec_game_selec.c
        src/view/screen/launch.c
        src/view/screen/mapping.c
        src/view/screen/settings.c
        src/view/screen/starting.c
        src/view/screen/platform_list.c
        src/view/screen/main_popup.c
        src/view/screen/exec_popup.c
        src/view/screen/simple_popup.c
)

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${MEHSTATION_SOURCES}
)

INCLUDE(FindPkgConfig)
IF(MINGW_BUILD)
  SET (PKG_CONFIG_EXECUTABLE i686-w64-mingw32-pkg-config)
ENDIF()


PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2_ttf REQUIRED SDL2_ttf)
PKG_SEARCH_MODULE(SDL2_image REQUIRED SDL2_image)
PKG_SEARCH_MODULE(sqlite3 REQUIRED sqlite3)
PKG_SEARCH_MODULE(libavcodec REQUIRED libavcodec)
PKG_SEARCH_MODULE(libavformat REQUIRED libavformat)
PKG_SEARCH_MODULE(libavutil REQUIRED libavutil)
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0>=2.0.0)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${SDL2_image_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${sqlite3_INCLUDE_DIRS}
    ${libavcodec_INCLUDE_DIRS}
    ${libavformat_INCLUDE_DIRS}
    ${libavutil_INCLUDE_DIRS}
)

include_directories(
${mehstation_BINARY_DIR}/src
)

LINK_DIRECTORIES(${mehstation_BINARY_DIR}/src)

CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h RASPBERRY)

IF(MINGW_BUILD)
  SET (CMAKE_C_COMPILER             "/usr/bin/i686-w64-mingw32-gcc")
  SET (CMAKE_C_FLAGS                "-Wall -std=c11")
  SET (CMAKE_C_FLAGS_DEBUG          "-g")
  ADD_DEFINITIONS(-DWINDOWS)
ELSE(UNIX)
  ADD_DEFINITIONS(-DLINUX)
  # Use Clang
  #SET (CMAKE_C_COMPILER             "clang")
  IF (RASPBERRY)
          SET (CMAKE_C_FLAGS                "-Wall -std=c99")
  ELSE()
          SET (CMAKE_C_FLAGS                "-Wall -std=c11")
  ENDIF()
  SET (CMAKE_C_FLAGS_DEBUG          "-g")
ENDIF()

# sources
TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${GLIB_LIBRARIES}
    ${sqlite3_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_ttf_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${libavcodec_LIBRARIES}
    ${libavformat_LIBRARIES}
    ${libavutil_LIBRARIES}
)

# -DSDL2_ttf_LIBRARIES=C:\Code\Libs\SDL2_ttf\lib\x86\SDL2_ttf.lib -DSDL2_image_LIBRARIES=C:\Code\Libs\SDL2_image\lib\x86\SDL2_image.lib -DSDL2_LIBRARIES=C:\Code\Libs\SDL2\SDL\lib\win32\SDL2main.lib;C:\Code\Libs\SDL2\SDL\lib\win32\SDL2.lib -DGLIB_LIBRARIES=C:\Code\Libs\glib\lib\libgio-2.0.dll.a;C:\Code\Libs\glib\lib\libglib-2.0.dll.a; -Dmehstation_BINARY_DIR=C:\\Code\\Projects\\mehstation -Dsqlite3_INCLUDE_DIRS=C:\Code\Libs\sqlite3 -DGLIB_INCLUDE_DIRS=C:\Code\Libs\glib\include;C:\Code\Libs\glib\include\glib-2.0\;C:\Code\Libs\glib\lib\glib-2.0\include -DSDL2_INCLUDE_DIRS=C:\Code\Libs\SDL2\SDL\include;C:\Code\Libs\SDL2\SDL\include\SDL2 -DSDL2_image_INCLUDE_DIRS=C:\Code\Libs\SDL2_image\include -DSDL2_ttf_INCLUDE_DIRS=C:\Code\Libs\SDL2_ttf\include
