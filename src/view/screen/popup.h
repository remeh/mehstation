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
struct Executable;

typedef struct {
	/* upon which screen the popup is appearing. */
	Screen* src_screen;

	struct Executable* executable;
	
	/* Widgets */
	WidgetRect* hover_widget;

	WidgetText* title_widget;

	WidgetText* favorite_widget;

	WidgetRect* selection_widget;

	int x;
	int y;

	int height;
	int width;

	int action;
} PopupData;

Screen* meh_screen_popup_new(App* app, Screen* src_screen, struct Executable* executable);
PopupData* meh_screen_popup_get_data(Screen* screen);
void meh_screen_popup_destroy_data(Screen* screen);
int meh_screen_popup_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_screen_popup_update(struct App* app, Screen* screen);
void meh_screen_popup_render(struct App* app, Screen* screen);

