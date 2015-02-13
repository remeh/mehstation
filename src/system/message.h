#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	int id;
	void* data;
} Message;

Message* meh_message_new(int message_id, void* data);
void meh_message_destroy(Message* message);
