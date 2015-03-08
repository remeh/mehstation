/*
 * mehstation - Starting screen.
 *
 * Copyright © 2015 Rémy Mathieu
 * 
 * TODO Rename to Splashscreen ?
 */

#pragma once

#include "system/message.h"
#include "view/screen.h"
#include "view/widget_image.h"

/* cross-reference */
struct App;

typedef struct {
	SDL_Texture* splash_texture;

	WidgetImage* splash;
} StartingData;

Screen* meh_screen_starting_new(struct App* app);
StartingData* meh_screen_starting_get_data(Screen* screen);
void meh_screen_starting_destroy_data(Screen* screen);
int meh_screen_starting_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_starting_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_starting_update(struct App* app, Screen* screen);
Screen* meh_screen_starting_create_next_screen(Screen* screen);
void meh_screen_starting_render(struct App* app, Screen* screen, gboolean flip);
