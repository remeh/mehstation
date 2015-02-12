#pragma once

#include "system/message.h"

/* cross-reference. */
struct App;

typedef struct Screen {
	struct Screen* parent_screen;
	gchar* name;
	int (*messages_handler) (struct App* app, struct Screen* screen, Message* message);
	int (*update) (struct Screen* screen, int delta_time);
	/* TODO list of texture */
} Screen;

void meh_screen_destroy(Screen* screen);
