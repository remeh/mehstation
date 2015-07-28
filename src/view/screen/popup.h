/*
 * mehstation - A popup hovering another screen.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"
#include "view/widget_rect.h"

#define MEH_FADE_STATE_IN 0
#define MEH_FADE_STATE_OUT 1

struct App;

typedef struct {
	/* upon which screen the popup is appearing. */
	Screen* src_screen;
	
	/* Widgets */
	WidgetRect* background_widget;
	WidgetRect* hover_widget;

	WidgetText* title_widget;
	WidgetRect* title_bg_widget;

	int x;
	int y;

	int height;
	int width;

} PopupData;

Screen* meh_screen_popup_new(App* app, Screen* src_screen, int width, int height, gchar* title);
PopupData* meh_screen_popup_get_data(Screen* screen);
void meh_screen_popup_destroy_data(Screen* screen);
int meh_screen_popup_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_screen_popup_update(struct App* app, Screen* screen);
void meh_screen_popup_render(struct App* app, Screen* screen);

