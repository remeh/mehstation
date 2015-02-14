#pragma once

#include "system/message.h"
#include "view/screen.h"

/* cross-reference */
struct App;

Screen* meh_screen_system_list_new();
int meh_screen_system_list_messages_handler(struct App* app, Screen* screen, Message* message);
void meh_screen_system_list_button_pressed(struct App* app, Screen* screen, int pressed_button);
int meh_screen_system_list_update(Screen* screen, int delta_time);
int meh_screen_system_list_render(struct App* app, Screen* screen);
