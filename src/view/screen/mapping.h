/*
 * mehstation - Screen to configure a mapping.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "system/message.h"
#include "view/screen.h"
#include "view/widget_image.h"

/* cross-reference */
struct App;

typedef struct {
	WidgetRect* overlay; /* black overlay */
} MappingData;

Screen* meh_screen_mapping_new(struct App* app);
MappingData* meh_screen_mapping_get_data(Screen* screen);
void meh_screen_mapping_destroy_data(Screen* screen);
int meh_screen_mapping_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_mapping_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_mapping_update(struct App* app, Screen* screen);
void meh_screen_mapping_render(struct App* app, Screen* screen, gboolean flip);
