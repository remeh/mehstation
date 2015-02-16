#pragma once

#include "system/db.h"
#include "system/input.h"
#include "system/message.h"
#include "system/settings.h"
#include "view/text.h"
#include "view/screen.h"
#include "view/window.h"

typedef struct {
	SDL_Event event;
	gboolean running;
} Mainloop;

typedef struct App {
	Window* window;
	Screen* current_screen;
	Font* small_font;
	DB* db;
	InputManager* input_manager;
	Settings settings;
	Mainloop mainloop;
} App;

App* meh_app_create();
int meh_app_init();
void meh_app_exit(App* app);
int meh_app_destroy(App* app);
void meh_app_set_current_screen(App* app, Screen* screen);
int meh_app_main_loop(App* app);
void meh_app_main_loop_event(App* app);
void meh_app_main_loop_update(App* app);
void meh_app_main_loop_render(App* app);
void meh_app_send_message(App* app, Message* message);
gboolean meh_app_open_or_create_db(App*  app);
