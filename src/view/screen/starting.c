#include "glib-2.0/glib.h"

#include "view/screen/starting.h"

Screen* meh_screen_starting_new() {
	Screen* screen = g_new(Screen, 1);

	screen->name = g_strdup("Starting screen");
	screen->events_handler = &meh_screen_starting_events_handler;

	return screen;
}

int meh_screen_starting_events_handler(struct Screen* screen, Event* event) {
	g_message("Received an event !");

	/* we consume it */
	g_free(event);

	return 0;
}

