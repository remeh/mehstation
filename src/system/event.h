#pragma once

typedef struct {
	int id;
} Event;

void meh_event_destroy(Event* event);
