#pragma once

#include "system/message.h"
#include "view/screen.h"
#include "view/window.h"

/* cross-reference */
struct App;

Screen* meh_screen_starting_new();
int meh_screen_starting_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_starting_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_starting_update(Screen* screen, int delta_time);
Screen* meh_screen_starting_create_next_screen(Screen* screen);
void meh_screen_starting_render(struct App* app, Screen* screen);
