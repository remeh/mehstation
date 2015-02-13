#include "glib-2.0/glib.h"
#include "message.h"

Message* meh_message_new(int message_id, void* data) {
	Message* message = g_new(Message, 1);
	message->id = message_id;
	message->data = data;
	return message;
}

void meh_message_destroy(Message* message) {
	if (message->data != NULL) {
		g_free(message->data);
		message->data = NULL;
	}
	g_free(message);
}
