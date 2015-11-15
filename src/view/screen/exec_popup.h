/*
 * mehstation - A popup hovering another screen.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"

struct App;

void meh_exec_popup_start_random_executable(App* app, Screen* screen);
void meh_exec_popup_favorite_toggle(struct App* app, Screen* screen);
