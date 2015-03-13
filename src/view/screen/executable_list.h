/*
 * mehstation - Screen of executables.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include <glib.h>

#include "system/message.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/widget_multi_text.h"

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
	int background; /* Index of the background in the textures cache */
	int cover; /* Index of the cover in the textures cache. */
	int screenshots[3]; /* Index of the first screenshot in the texture cache. */

	/*
	 * Widgets
	 */
	WidgetRect* selection_widget;
	WidgetRect* background_hover_widget;
	WidgetRect* header_widget;
	WidgetRect* list_background_widget;
	WidgetText* header_text_widget;

	WidgetImage* cover_widget;
	WidgetImage* background_widget;
	WidgetImage* screenshots_widget[3];

	WidgetText* genres_widget;
	WidgetText* genres_l_widget;

	WidgetText* players_widget;
	WidgetText* players_l_widget;

	WidgetText* publisher_widget;
	WidgetText* publisher_l_widget;

	WidgetText* developer_widget;
	WidgetText* developer_l_widget;

	WidgetText* rating_widget;
	WidgetText* rating_l_widget;

	WidgetText* release_date_widget;
	WidgetText* release_date_l_widget;

	WidgetMultiText* description_widget;

	GQueue* executable_widgets;
} ExecutableListData;

Screen* meh_screen_exec_list_new(struct App* app, int platform_id);
void meh_screen_exec_list_destroy_data();
int meh_screen_exec_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_exec_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_exec_list_update(Screen* screen);;
int meh_screen_exec_list_render(struct App* app, Screen* screen, gboolean flip);
ExecutableListData* meh_screen_exec_list_get_data(Screen* screen);
