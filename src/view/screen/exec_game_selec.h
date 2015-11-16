#pragma once

#include "system/app.h"
#include "view/screen.h"

struct ExecutableListData;

void meh_game_selec_create_widgets(App* app, Screen* screen);
void meh_game_selec_destroy(Screen* screen);
void meh_game_selec_render(App* app, Screen* screen);
void meh_game_selec_refresh_executables_widgets(App* app, Screen* screen);
