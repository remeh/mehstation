/*
 * mehstation - Screen to configure a mapping.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "system/message.h"
#include "view/screen.h"
#include "view/widget_image.h"

#define MEH_MAPPING_STEP_IDENTIFY 0
#define MEH_MAPPING_STEP_UP 1
#define MEH_MAPPING_STEP_DOWN 2
#define MEH_MAPPING_STEP_LEFT 3
#define MEH_MAPPING_STEP_RIGHT 4
#define MEH_MAPPING_STEP_A 5
#define MEH_MAPPING_STEP_B 6
#define MEH_MAPPING_STEP_L 7
#define MEH_MAPPING_STEP_R 8
#define MEH_MAPPING_STEP_START 9
#define MEH_MAPPING_STEP_SELECT 10
#define MEH_MAPPING_STEP_END 11

/* cross-reference */
struct App;

typedef struct {
	int step;
	WidgetText* title;
	WidgetText* device_configuring;
	WidgetText* action;

	int up;
	int down;
	int right;
	int left;
	int a;
	int b;
	int l;
	int r;
	int start;
	int select;
} MappingData;

Screen* meh_screen_mapping_new(struct App* app);
MappingData* meh_screen_mapping_get_data(Screen* screen);
void meh_screen_mapping_destroy_data(Screen* screen);
int meh_screen_mapping_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_mapping_button_pressed(struct App* app, Screen* screen, int pressed_button, int sdl_key, gchar* guid);
int meh_screen_mapping_update(struct App* app, Screen* screen);
void meh_screen_mapping_render(struct App* app, Screen* screen, gboolean flip);
