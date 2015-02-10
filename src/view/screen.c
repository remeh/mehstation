#include <stdlib.h>
#include "glib-2.0/glib.h"

#include "view/screen.h"

void meh_screen_destroy(Screen* screen) {
	g_assert(screen != NULL);
	g_free(screen->name);
}
