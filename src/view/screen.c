#include <stdlib.h>
#include <glib.h>

#include "system/transition.h"
#include "view/screen.h"

Screen* meh_screen_new() {
	Screen* screen = g_new(Screen, 1);
	screen->parent_screen = NULL;
	screen->data = NULL;
	screen->transitions = g_queue_new();
	screen->destroy_data = NULL;
	return screen;
}

void meh_screen_destroy(Screen* screen) {
	g_assert(screen != NULL);

	if (screen->data != NULL) {
		if (screen->destroy_data != NULL) {
			screen->destroy_data(screen);
		} else {
			g_warning("No destroy_data on Screen '%s', but the data isn't NULL!", screen->name);
		}
	}

	g_queue_free(screen->transitions);
	g_free(screen->name);
	g_free(screen);
}
