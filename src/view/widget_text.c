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
	t->texture = NULL;

	return t;
}

void meh_widget_text_destroy(WidgetText* text) {
	g_assert(text != NULL);

	if (text->texture != NULL) {
		SDL_DestroyTexture(text->texture);
	}

	g_free(text);
}

/*
 * meh_widget_text_reload writes the text on a texture and store it in the WidgetText.
 */
void meh_widget_text_reload(Window* window, WidgetText* text) {
	if (text->texture != NULL) {
		SDL_DestroyTexture(text->texture);
	}

	SDL_Color color = {
		text->r.value,
		text->g.value,
		text->b.value,
		text->a.value,
	};

	text->texture = meh_window_render_text_texture(window, text->font, text->text, color);

	SDL_QueryTexture(text->texture, NULL, NULL, &text->tex_w, &text->tex_h);
	g_debug("Texture for text %s loaded.", text->text);
}

void meh_widget_text_render(Window* window, WidgetText* text) {
	g_assert(text != NULL);
	g_assert(window != NULL);

	/* Don't render empty text. */
	if (text->text == NULL || strlen(text->text) == 0) {
		return;
	}

	if (text->texture == NULL) {
		meh_widget_text_reload(window, text);
	}


	SDL_Rect position = { 
		meh_window_convert_width(window, text->x.value),
		meh_window_convert_height(window, text->y.value),
		text->tex_w,
		text->tex_h
	};
	
	meh_window_render_texture(window, text->texture, position);
}
