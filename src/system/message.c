#include "glib-2.0/glib.h"
#include "message.h"
#include "app.h"
#include "view/screen.h"

Message* meh_message_new(int message_id, void* data) {
	Message* message = g_new(Message, 1);
	message->id = message_id;
	message->data = data;
	return message;
}

/*
 * meh_message_send sends a message to the given screen.
 */
void meh_message_send(App* app, Screen* screen, int msg_type, void* data) {
	g_assert(screen != NULL);

	Message* message = meh_message_new(msg_type, data);
	screen->messages_handler(app, screen, message);
	meh_message_destroy(message);
}

void meh_message_destroy(Message* message) {
	if (message->data != NULL) {
		g_free(message->data);
		message->data = NULL;
	}
	g_free(message);
}
