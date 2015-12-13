/*
 * mehstation - Launch transition.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"
#include "system/db/models.h"

struct App;

typedef struct {
	Screen* src_screen;

	Executable* executable;
	Platform* platform;
	
	/* will we do a zoom on the logo ? */
	gboolean zoom_logo;

	/* Widgets */
	WidgetRect* fade_widget;

	/* original image widget, pointer to the widget in the executable list screen. Do not free. */
	WidgetImage* src_widget;

	/* the cover we'll zoom on */
	WidgetImage* image_widget;
} LaunchData;

Screen* meh_screen_launch_new(struct App* app, Screen* src_screen, Platform* platform, Executable* executable,
								WidgetImage* src_widget);
LaunchData* meh_screen_launch_get_data(Screen* screen);
void meh_screen_launch_destroy_data(Screen* screen);
int meh_screen_launch_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_screen_launch_update(struct App* app, Screen* screen);
void meh_screen_launch_render(struct App* app, Screen* screen);

