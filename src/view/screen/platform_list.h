#pragma once

#include <glib.h>

#include "system/message.h"
#include "system/transition.h"
#include "view/screen.h"
#include "view/widget_text.h"

/* cross-reference */
struct App;

typedef struct PlatformListData {
	GQueue* platforms;
	int selected_platform;

	/*
	 * Widgets
	 */
	WidgetText* no_platforms_widget;
	WidgetText* title;
	WidgetRect* selection_widget;

	GQueue* platforms_icons; /* Queue of SDL_Texture*, memory must be freed */
	GQueue* icons_widgets; /* List of WidgetImage*, memory must be freed */
} PlatformListData;

Screen* meh_screen_platform_list_new(struct App* app);
void meh_screen_platform_list_destroy_data();
int meh_screen_platform_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_platform_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_platform_list_update(Screen* screen);;
int meh_screen_platform_list_render(struct App* app, Screen* screen, gboolean flip);
PlatformListData* meh_screen_platform_list_get_data(Screen* screen);
