#pragma once

#include "system/message.h"

/* cross-reference. */
struct App;

typedef struct Screen {
	struct Screen* parent_screen;
	gchar* name;
	int (*messages_handler) (struct App* app, struct Screen* screen, Message* message);
	/* TODO list of texture */
} Screen;

void meh_screen_destroy(Screen* screen);
