/*
 * mehstation - Fading transition.
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
	Screen* src_screen;
	Screen* dst_screen;

	int state;
	
	/* Widgets */
	WidgetRect* fade_widget;
} FadeData;

Screen* meh_screen_fade_new(App* app, Screen* src_screen, Screen* dst_screen);
FadeData* meh_screen_fade_get_data(Screen* screen);
void meh_screen_fade_destroy_data(Screen* screen);
int meh_screen_fade_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_screen_fade_update(struct App* app, Screen* screen);
void meh_screen_fade_render(struct App* app, Screen* screen);
