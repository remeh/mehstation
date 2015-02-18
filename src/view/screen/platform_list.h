#pragma once

#include "system/message.h"
#include "view/screen.h"

/* cross-reference */
struct App;

typedef struct PlatformListData {
	GSList* platforms;
	int selected_platform;
} PlatformListData;

Screen* meh_screen_platform_list_new(struct App* app);
void meh_screen_platform_list_destroy_data();
int meh_screen_platform_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_platform_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_platform_list_update(Screen* screen, int delta_time);
int meh_screen_platform_list_render(struct App* app, Screen* screen);
PlatformListData* meh_screen_platform_list_get_data(Screen* screen);
