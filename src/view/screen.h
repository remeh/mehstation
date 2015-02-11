#pragma once

#include "system/event.h"

typedef struct Screen {
	struct Screen* parent_screen;
	gchar* name;
	int (*events_handler) (struct Screen* screen, Event* event);
	int (*update) (struct Screen* screen, int delta_time);
	/* TODO list of texture */
} Screen;

void meh_screen_destroy(Screen* screen);
