
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(mehstation)

SET(
        MEHSTATION_SOURCES
        src/main.c
        src/system/app.c
        src/system/db.c
        src/system/input.c
        src/system/message.c
        src/system/settings.c
        src/system/transition.c
        src/system/db/platform.c
        src/system/db/executable.c
        src/system/db/executable_resource.c
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
        src/view/screen/starting.c
        src/view/screen/platform_list.c
)

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${MEHSTATION_SOURCES}
)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2_ttf REQUIRED SDL2_ttf)
PKG_SEARCH_MODULE(SDL2_image REQUIRED SDL2_image)
PKG_SEARCH_MODULE(sqlite3 REQUIRED sqlite3)
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0>=2.0.0)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${SDL2_image_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${sqlite3_INCLUDE_DIRS}
    ${mehstation_BINARY_DIR}/src
)

LINK_DIRECTORIES(${mehstation_BINARY_DIR}/src)

# Use Clang
SET (CMAKE_C_COMPILER             "/usr/bin/clang")
SET (CMAKE_C_FLAGS                "-Wall -std=c11")
SET (CMAKE_C_FLAGS_DEBUG          "-g")

# sources
TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${SDL2_ttf_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${sqlite3_LIBRARIES}
)

