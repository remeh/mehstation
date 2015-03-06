/*
 * mehstation - Fading transition.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

struct App;

typedef struct {
	Screen* next_screen;

} FadeData;

Screen* meh_screen_fade_new(struct App* app);
void meh_screen_fade_destroy(Screen* screen);
FadeData* meh_screen_fade_get_data(Screen* screen);
void meh_screen_fade_destroy_data(Screen* screen);
int meh_screen_fade_update(struct App* app, Screen* screen, int delta_time);
void meh_screen_fade_render(struct App* app, Screen* screen);
