/*
 * mehstation - Multi text widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/text.h"
#include "view/widget_text.h"
#include "view/window.h"
#include "system/transition.h"

typedef struct {
	/* embed a text widget */
	WidgetText* widget_text;

	float max_width;
} WidgetMultiText;

WidgetMultiText* meh_widget_multi_text_new(const Font* font, const char* text, float x, float y, SDL_Color color, gboolean shadow, float max_width);
void meh_widget_multi_text_destroy(WidgetMultiText* text);
void meh_widget_multi_text_render(Window* window, WidgetMultiText* text);
void meh_widget_multi_text_reload(Window* window, WidgetMultiText* text);

