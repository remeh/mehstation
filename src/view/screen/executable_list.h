#pragma once

#include <glib.h>

#include "system/message.h"
#include "view/screen.h"
#include "view/widget_rect.h"

/* cross-reference */
struct App;

typedef struct ExecutableListData {
	Platform* platform;
	GQueue* executables; /* List of Executable*, must be freed. */
	int executables_length;
	int selected_executable;
	GHashTable* textures; /* Hash int->SDL_Texture*, each SDL_Texture* must be freed. */

	GQueue *cache_executables_id; /* Contains the executables for which we have load the resources
									 The first loaded is the first in the queue. */
	int background; /* Reference to a texture in the map, should NOT be freed ! */
	int cover; /* Reference to a texture in the map, shouldn't be freed! */

	/*
	 * Widgets
	 */
	WidgetRect* selection_widget;
	WidgetRect* header_widget;
	WidgetRect* list_background_widget;
	WidgetText* header_text_widget;
	WidgetImage* cover_widget;
	WidgetImage* background_widget;
} ExecutableListData;

Screen* meh_screen_exec_list_new(struct App* app, int platform_id);
void meh_screen_exec_list_destroy_data();
int meh_screen_exec_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_exec_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_exec_list_update(Screen* screen, int delta_time);
int meh_screen_exec_list_render(struct App* app, Screen* screen);
ExecutableListData* meh_screen_exec_list_get_data(Screen* screen);
