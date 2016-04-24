/*
 * mehstation - debug input screen.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"
#include "view/widget_rect.h"

/* cross-reference */
struct App;

typedef struct {
	WidgetRect** states;
} DebugInputData;

Screen* meh_screen_debug_input_new(struct App* app);
void meh_screen_debug_input_destroy_data(Screen* screen);
int meh_screen_debug_input_messages_handler(struct App* app, Screen* screen, Message* message);
DebugInputData* meh_screen_debug_input_get_data(Screen* screen);
void meh_screen_debug_input_render(struct App* app, Screen* screen, gboolean flip);
void meh_screen_debug_input_button_pressed(struct App* app, Screen* screen, int pressed_button);
void meh_screen_debug_input_update(struct App* app, Screen* screen);
