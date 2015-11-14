/*
 * mehstation - The platform list popup (with shutdown, etc.)
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"

struct App;

void meh_main_popup_os_shutdown(struct App* app, Screen* screen);
void meh_main_popup_close(struct App* app, Screen* screen);
void meh_main_popup_random_executable(struct App* app, Screen* screen);
