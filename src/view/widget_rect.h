/*
 * mehstation - Rect widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include <SDL2/SDL.h>
#include <glib.h>

#include "view/image.h"
#include "view/window.h"
#include "system/transition.h"

typedef struct {
	/* Pos */
	Transition x;
	Transition y;
	/* Size*/
	Transition w;
	Transition h;
	/* Color */
	Transition r;
	Transition g;
	Transition b;
	Transition a;

	gboolean filled;
} WidgetRect;

WidgetRect* meh_widget_rect_new(int x, int y, int w, int h, SDL_Color color, gboolean filled);
void meh_widget_rect_destroy(WidgetRect* rect);
void meh_widget_rect_render(Window* window, const WidgetRect* rect);
