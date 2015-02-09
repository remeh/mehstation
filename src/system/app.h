#pragma once

#include "system/settings.h"
#include "view/window.h"
#include "view/text.h"

typedef struct {
	Window* window;
	Font* small_font;
	Settings settings;
} App;

App* meh_app_create();
int meh_app_init();
int meh_app_destroy();

