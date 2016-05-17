/*
 * mehstation - Image widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include <SDL2/SDL.h>

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

	/* On which texture this widget is pointing. Do not free this pointer. */
	SDL_Texture* texture;
} WidgetImage;

WidgetImage* meh_widget_image_new(SDL_Texture* texture, float x, float y, float w, float h);
void meh_widget_image_destroy(WidgetImage* image);
void meh_widget_image_render(Window* window, const WidgetImage* image);
