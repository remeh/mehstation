#include <stdlib.h>
#include "glib-2.0/glib.h"

#include "view/screen.h"

Screen* meh_screen_new() {
	Screen* screen = g_new(Screen, 1);
	screen->parent_screen = NULL;
	return screen;
}

void meh_screen_destroy(Screen* screen) {
	g_assert(screen != NULL);
	g_free(screen->name);
}
