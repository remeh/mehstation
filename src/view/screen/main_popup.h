/*
 * mehstation - The platform list popup (with shutdown, etc.)
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"
#include "view/widget_rect.h"

struct App;
struct Executable;

typedef struct {
	/* upon which screen the popup is appearing. */
	Screen* src_screen;
	
	/* Widgets */
	WidgetRect* hover_widget;

	WidgetText* title_widget;

	WidgetText* favorite_widget;
	WidgetText* random_widget;

	WidgetRect* selection_widget;

	int x;
	int y;

	int height;
	int width;

	int action;

	gboolean quitting;

} MainPopupData;

Screen* meh_main_popup_new(App* app, Screen* src_screen);
MainPopupData* meh_main_popup_get_data(Screen* screen);
void meh_main_popup_destroy_data(Screen* screen);
int meh_main_popup_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_main_popup_update(struct App* app, Screen* screen);
void meh_main_popup_render(struct App* app, Screen* screen);
void meh_main_popup_random_executable(struct App* app);
