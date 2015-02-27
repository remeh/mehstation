/*
 * mehstation - Text widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/text.h"
#include "view/window.h"
#include "system/transition.h"

typedef struct {
	gchar* text;	
	const Font* font; /* Reference to the font to use, shouldn't be freed. */

	/* Pos */
	Transition x;
	Transition y;

	/* Color */
	Transition r;
	Transition g;
	Transition b;
} Text;

Text* meh_widget_text_new(const Font* font, const char* text, int x, int y, SDL_Color color);
void meh_widget_text_destroy(Text* text);
void meh_widget_text_render(const Text* text, const Window* window);
