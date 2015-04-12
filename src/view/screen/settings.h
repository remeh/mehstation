/*
 * mehstation - Settings screen.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"
#include "view/widget_rect.h"

struct App;

typedef struct {
} SettingsData;

Screen* meh_screen_settings_new(App* app);
SettingsData* meh_screen_settings_get_data(Screen* screen);
void meh_screen_settings_destroy_data(Screen* screen);
int meh_screen_settings_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_screen_settings_update(struct App* app, Screen* screen);
void meh_screen_settings_render(struct App* app, Screen* screen, gboolean flip);

