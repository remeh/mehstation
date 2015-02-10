#pragma once

#include "system/event.h"

typedef struct {
	struct Screen* parent_screen;
	gchar* name;
	int (*events_handler) (struct Screen* screen, Event* event);
	/* TODO list of texture */
} Screen;

void meh_screen_destroy(Screen* screen);
