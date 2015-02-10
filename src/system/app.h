#pragma once

#include "system/event.h"
#include "system/settings.h"
#include "view/text.h"
#include "view/screen.h"
#include "view/window.h"

typedef struct {
	SDL_Event event;
	gboolean running;
} Mainloop;

typedef struct {
	Window* window;
	Screen* current_screen;
	Font* small_font;
	Settings settings;
	Mainloop mainloop;
} App;

App* meh_app_create();
int meh_app_init();
int meh_app_destroy(App* app);
void meh_app_set_current_screen(App* app, Screen* screen);
int meh_app_main_loop(App* app);
void meh_app_main_loop_event(App* app);
void meh_app_main_loop_render(App* app);
void meh_app_send_event(App* app, Event* event);
