#pragma once

#include "system/message.h"
#include "view/screen.h"

/* cross-reference */
struct App;

Screen* meh_screen_starting_new();
int meh_screen_starting_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_starting_button_pressed(struct App* app, Screen* screen, Message* message);
int meh_screen_starting_update(Screen* screen, int delta_time);
