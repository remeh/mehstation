/*
 * mehstation - Text widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "view/text.h"
#include "view/widget_text.h"
#include "view/window.h"

WidgetText* meh_widget_text_new(const Font* font, const char* text, float x, float y, SDL_Color color, gboolean shadow) {
	WidgetText* t = g_new(WidgetText, 1);

	t->font = font;
	t->text = g_strdup(text);
	
	t->x = meh_transition_start(MEH_TRANSITION_NONE, x, x, 0);
	meh_transition_end(&t->x);
	t->y = meh_transition_start(MEH_TRANSITION_NONE, y, y, 0);
	meh_transition_end(&t->y);

	t->r = meh_transition_start(MEH_TRANSITION_NONE, color.r, color.r, 0);
	meh_transition_end(&t->r);
	t->g = meh_transition_start(MEH_TRANSITION_NONE, color.g, color.g, 0);
	meh_transition_end(&t->g);
	t->b = meh_transition_start(MEH_TRANSITION_NONE, color.b, color.b, 0);
	meh_transition_end(&t->b);
	t->a = meh_transition_start(MEH_TRANSITION_NONE, color.a, color.a, 0);
	meh_transition_end(&t->a);

	t->shadow = shadow;

	return t;
}

void meh_widget_text_destroy(WidgetText* text) {
	g_assert(text != NULL);

	g_free(text);
}

void meh_widget_text_render(Window* window, const WidgetText* text) {
	g_assert(text != NULL);
	g_assert(window != NULL);

	/* Don't render empty text. */
	if (text->text == NULL || strlen(text->text) == 0) {
		return;
	}

	float x = meh_window_convert_width(window, text->x.value);
	float y = meh_window_convert_height(window, text->y.value);

	SDL_Color color = { text->r.value, text->g.value, text->b.value , text->a.value };
	if (text->shadow) {
		SDL_Color black = { 0, 0, 0 };
		// FIXME +4.0f not normalized
		meh_window_render_text(window, text->font, text->text, black, x+4.0f, y+4.0f); /* shadow */
	}

	meh_window_render_text(window, text->font, text->text, color, x, y); /* text */
}
