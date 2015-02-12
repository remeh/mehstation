#include "glib-2.0/glib.h"
#include "event.h"

void meh_event_destroy(Event* event) {
	g_free(event);
}
