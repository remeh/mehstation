#pragma once

#include <glib-2.0/glib.h>

#include "system/message.h"
#include "view/screen.h"

/* cross-reference */
struct App;

typedef struct ExecutableListData {
	Platform* platform;
	GSList* executables;
	int selected_executable;
} ExecutableListData;

Screen* meh_screen_executable_list_new(struct App* app, int platform_id);
void meh_screen_executable_list_destroy_data();
int meh_screen_executable_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_executable_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_executable_list_update(Screen* screen, int delta_time);
int meh_screen_executable_list_render(struct App* app, Screen* screen);
ExecutableListData* meh_screen_executable_list_get_data(Screen* screen);
