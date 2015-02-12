#include "glib-2.0/glib.h"
#include "message.h"

void meh_message_destroy(Message* message) {
	g_free(message);
}
