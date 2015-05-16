CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(mehstation)

SET(
        MEHSTATION_SOURCES
        src/main.c
        src/system/app.c
        src/system/db.c
        src/system/flags.c
        src/system/input.c
        src/system/message.c
        src/system/settings.c
        src/system/transition.c
        src/system/db/executable.c
        src/system/db/executable_resource.c
        src/system/db/mapping.c
        src/system/db/platform.c
        src/view/image.c
        src/view/screen.c
        src/view/text.c
        src/view/window.c
        src/view/widget_image.c
        src/view/widget_rect.c
        src/view/widget_text.c
        src/view/screen/executable_list.c
        src/view/screen/fade.c
        src/view/screen/mapping.c
        src/view/screen/settings.c
        src/view/screen/starting.c
        src/view/screen/platform_list.c
)

IF(WIN32)
  SET(
    src/ext/sqlite3.c
  )
ENDIF()

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${MEHSTATION_SOURCES}
)

IF(UNIX) # Use pkg-config only on unix systems.
  INCLUDE(FindPkgConfig)
  PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
  PKG_SEARCH_MODULE(SDL2_ttf REQUIRED SDL2_ttf)
  PKG_SEARCH_MODULE(SDL2_image REQUIRED SDL2_image)
  PKG_SEARCH_MODULE(sqlite3 REQUIRED sqlite3)
  PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0>=2.0.0)
ENDIF()

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${SDL2_image_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${sqlite3_INCLUDE_DIRS}
)

IF(UNIX)
  include_directories(
    ${mehstation_BINARY_DIR}/src
  )
ELSEIF(WIN32)
  include_directories(
    ${mehstation_BINARY_DIR}\\src
  )
ENDIF()

LINK_DIRECTORIES(${mehstation_BINARY_DIR}/src)

IF(UNIX)
  # Use Clang
  SET (CMAKE_C_COMPILER             "/usr/bin/clang")
  SET (CMAKE_C_FLAGS                "-Wall -std=c11")
  SET (CMAKE_C_FLAGS_DEBUG          "-g")
ENDIF()

# sources
TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${SDL2_ttf_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${sqlite3_LIBRARIES}
)

# -DSDL2_ttf_LIBRARIES=C:\Code\Libs\SDL2_ttf\lib\x86\SDL2_ttf.lib -DSDL2_image_LIBRARIES=C:\Code\Libs\SDL2_image\lib\x86\SDL2_image.lib -DSDL2_LIBRARIES=C:\Code\Libs\SDL2\SDL\lib\win32\SDL2main.lib;C:\Code\Libs\SDL2\SDL\lib\win32\SDL2.lib -DGLIB_LIBRARIES=C:\Code\Libs\glib\lib\libgio-2.0.dll.a;C:\Code\Libs\glib\lib\libglib-2.0.dll.a; -Dmehstation_BINARY_DIR=C:\\Code\\Projects\\mehstation -Dsqlite3_INCLUDE_DIRS=C:\Code\Libs\sqlite3 -DGLIB_INCLUDE_DIRS=C:\Code\Libs\glib\include;C:\Code\Libs\glib\include\glib-2.0\;C:\Code\Libs\glib\lib\glib-2.0\include -DSDL2_INCLUDE_DIRS=C:\Code\Libs\SDL2\SDL\include;C:\Code\Libs\SDL2\SDL\include\SDL2 -DSDL2_image_INCLUDE_DIRS=C:\Code\Libs\SDL2_image\include -DSDL2_ttf_INCLUDE_DIRS=C:\Code\Libs\SDL2_ttf\include
