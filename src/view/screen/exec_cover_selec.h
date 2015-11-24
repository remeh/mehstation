/*
 * mehstation - Executable selection in the executables list.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#pragma once

#include "view/screen.h"

struct App;

void meh_cover_selec_create_widgets(struct App* app, Screen* screen);
void meh_cover_selec_destroy(Screen* screen);
void meh_cover_selec_update(Screen* screen);
void meh_cover_selec_render(struct App* app, Screen* screen);
void meh_cover_selec_adapt_view(App* app, Screen* screen, int prev_selected_id);
