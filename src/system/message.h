#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	int id;
	gpointer data;
} Message;

void meh_message_destroy(Message* message);
