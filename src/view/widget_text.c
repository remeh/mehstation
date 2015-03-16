/*
 * mehstation - Text widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "view/text.h"
#include "view/widget_text.h"
#include "view/window.h"
#include "view/screen.h"

WidgetText* meh_widget_text_new(const Font* font, const char* text, int x, int y, int w, int h, SDL_Color color, gboolean shadow) {
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

	t->start_timestamp = -1;
	t->off_x = 0;
	t->off_y = 0;

	t->w = w;
	t->h = h;

	/* By default, not uppercase */
	t->uppercase = FALSE;

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

	gchar* to_render = text->text;
	if (text->uppercase) {
		to_render = g_utf8_strup(text->text, -1);
	}

	/* Render the text on a texture. */
	text->texture = meh_window_render_text_texture(window, text->font, to_render, color, -1.0f);

	if (text->uppercase) {
		g_free(to_render);
	}

	SDL_QueryTexture(text->texture, NULL, NULL, &text->tex_w, &text->tex_h);
	g_debug("Texture for text %s loaded.", text->text);

	/* restart the movement infos */
	text->start_timestamp = -1;
	text->off_x = 0;
	text->off_y = 0;
}

void meh_widget_text_update(Screen* screen, WidgetText* text) {
	g_assert(screen != NULL);
	g_assert(text != NULL);

	if (text->start_timestamp == -1 && (text->tex_w > text->w || text->tex_h > text->h)) {
		text->start_timestamp = SDL_GetTicks() + MEH_TEXT_MOVING_AFTER;
	}

	if (SDL_GetTicks() > text->start_timestamp) {
		if (text->tex_w > text->w && (text->off_x + text->w) < text->tex_w) {
			text->off_x += 0.2f;
		}
		if (text->tex_h > text->h && (text->off_y + text->h) < text->tex_h) {
			text->off_y += 0.2f;
		}
	}
}

void meh_widget_text_render(Window* window, WidgetText* text) {
	g_assert(text != NULL);
	g_assert(window != NULL);

	if (text->texture == NULL) {
		meh_widget_text_reload(window, text);
	}

	/* compute the src rect to use in the texture */
	int src_x, src_y, src_w, src_h;
	src_x = text->off_x;
	src_y = text->off_y;
	src_w = text->tex_w;
	src_h = text->tex_h;

	if (text->tex_w > text->w) {
		src_w = text->w;
	}
	if (text->tex_h > text->h) {
		src_h = text->h;
	}

	SDL_Rect src = {
		src_x,
		src_y,
		src_w,
		src_h
	};


	/* dst target */

	/* don't render to large */
	int dst_w = text->w;
	int dst_h = text->h;
	if (text->tex_w < text->w) {
		dst_w = text->tex_w;
	}
	if (text->tex_h < text->h) {
		dst_h = text->tex_h;
	}

	SDL_Rect dst = { 
		meh_window_convert_width(window, text->x.value),
		meh_window_convert_height(window, text->y.value),
		meh_window_convert_width(window, dst_w),
		meh_window_convert_height(window, dst_h)
	};

	/* draw */
	meh_window_render_texture(window, text->texture, &src, &dst);
}
