/*
 * mehstation - Multi text widget.
 * Delegate a big part of the work
 * to WidgetText.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "view/text.h"
#include "view/widget_multi_text.h"
#include "view/window.h"

WidgetMultiText* meh_widget_multi_text_new(const Font* font, const char* text, float x, float y, SDL_Color color, gboolean shadow, float max_width) {
	WidgetMultiText* t = g_new(WidgetMultiText, 1);

	/* create the widget text */
	t->widget_text = meh_widget_text_new(font, text, x, y, color, shadow);

	t->max_width = max_width;

	return t;
}

void meh_widget_multi_text_destroy(WidgetMultiText* text) {
	g_assert(text != NULL);

	meh_widget_text_destroy(text->widget_text);

	g_free(text);
}

/*
 * meh_widget_text_reload writes the text on a texture and store it in the WidgetMultiText.
 */
void meh_widget_multi_text_reload(Window* window, WidgetMultiText* text) {
	/* this is the part changing compared to WidgetMultiText,
	 * the rendering of the texture */
	if (text->widget_text->texture != NULL) {
		SDL_DestroyTexture(text->widget_text->texture);
	}

	/* build the color */
	SDL_Color color = {
		text->widget_text->r.value,
		text->widget_text->g.value,
		text->widget_text->b.value,
		text->widget_text->a.value,
	};

	text->widget_text->texture = meh_window_render_text_texture(window, text->widget_text->font, text->widget_text->text, color, meh_window_convert_width(window, text->max_width));

	SDL_QueryTexture(text->widget_text->texture, NULL, NULL, &text->widget_text->tex_w, &text->widget_text->tex_h);
	g_debug("Texture for text %s loaded.", text->widget_text->text);
}

void meh_widget_multi_text_render(Window* window, WidgetMultiText* text) {
	g_assert(text != NULL);
	g_assert(window != NULL);

	meh_widget_text_render(window, text->widget_text);
}
