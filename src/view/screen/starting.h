#pragma once

#include "system/event.h"
#include "view/screen.h"

Screen* meh_screen_starting_new();
int meh_screen_starting_events_handler(Screen* screen, Event* event);
int meh_screen_starting_update(Screen* screen, int delta_time);
