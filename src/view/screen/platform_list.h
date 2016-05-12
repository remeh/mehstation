#pragma once

#include <glib.h>

#include "system/message.h"
#include "system/transition.h"
#include "view/screen.h"
#include "view/widget_text.h"
#include "view/widget_video.h"

/* cross-reference */
struct App;

struct lastPlayed {
	Executable* executable;
	Platform* platform;
	WidgetImage* widget_icon;
	SDL_Texture* icon;
};

typedef struct PlatformListData {
	GQueue* platforms;
	struct lastPlayed last_played;
	int selected_platform;

	/*
	 * Widgets
	 */
	WidgetText* no_platforms_widget;
	WidgetText* title;

	SDL_Texture* background;
	WidgetImage* background_widget;

	WidgetRect* background_hover;
	WidgetRect* hover;
	WidgetText* maintext;
	WidgetText* subtext;

	GQueue* platforms_icons; /* Queue of SDL_Texture*, memory must be freed */
	GQueue* icons_widgets; /* List of WidgetImage*, memory must be freed */
} PlatformListData;

Screen* meh_screen_platform_list_new(struct App* app);
void meh_screen_platform_list_destroy_data();
int meh_screen_platform_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_platform_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_platform_list_update(Screen* screen);
int meh_screen_platform_list_render(struct App* app, Screen* screen, gboolean flip);
PlatformListData* meh_screen_platform_list_get_data(Screen* screen);
void meh_screen_platform_change_platform(struct App* app, Screen* screen);
