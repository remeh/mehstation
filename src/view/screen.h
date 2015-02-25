#pragma once

#include <glib.h>
#include "system/message.h"

/* cross-reference. */
struct App;

typedef struct Screen {
	struct Screen* parent_screen;
	gchar* name;
	GQueue* transitions; /* the content of this queue must NOT be freed */
	int (*messages_handler) (struct App* app, struct Screen* screen, Message* message);
	/* extra data of the screen, if any, a destroy_data method must be attached. */
	void* data;
	void (*destroy_data) ();
} Screen;

Screen* meh_screen_new();
void meh_screen_destroy(Screen* screen);
void meh_screen_destroy_data_stub(Screen* screen);
