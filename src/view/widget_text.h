/*
 * mehstation - Text widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/text.h"
#include "view/window.h"
#include "system/transition.h"

#define MEH_TEXT_MOVING_AFTER 3000 /* after how many seconds the text should move if too long */

struct Screen;

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
	Transition a;

	gboolean shadow;
	gboolean uppercase;
	gboolean multi;

	/* which part of the text texture we wanna render */
	SDL_Rect srcrect;
	int w;
	int h;

	unsigned int start_timestamp;
	unsigned int restart_timestamp;
	float off_x;
	float off_y;

	/* At first rendering, the texture is cached for performance purpose.
	 * Use meh_widget_text_reload to refresh the texture. */
	SDL_Texture* texture;
	int tex_w;
	int tex_h;

} WidgetText;

WidgetText* meh_widget_text_new(const Font* font, const char* text, int x, int y, int w, int h, SDL_Color color, gboolean shadow);
void meh_widget_text_destroy(WidgetText* text);
void meh_widget_text_render(Window* window, WidgetText* text);
void meh_widget_text_reload(Window* window, WidgetText* text);
void meh_widget_text_reset_move(WidgetText* text);
void meh_widget_text_update(struct Screen* screen, WidgetText* text);
