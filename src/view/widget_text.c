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
	t->restart_timestamp = -1;
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
	g_debug("Texture for text %s loaded (%dx%d).", text->text, text->tex_w, text->tex_h);

	/* restart the movement infos */
	meh_widget_text_reset_move(text);
}

void meh_widget_text_update(Screen* screen, WidgetText* text) {
	g_assert(screen != NULL);
	g_assert(screen->window != NULL);
	g_assert(text != NULL);

	int c_text_w = meh_window_convert_width(screen->window, text->w);
	int c_text_h = meh_window_convert_width(screen->window, text->h);

	/* do we need to restart the move ? */
	if (text->restart_timestamp != -1 && SDL_GetTicks() > text->restart_timestamp) {
		meh_widget_text_reset_move(text);
	}

	/* do we need to start the move ? */
	if (text->start_timestamp == -1 && (text->tex_w > c_text_w || text->tex_h > c_text_h)) {
		text->start_timestamp = SDL_GetTicks() + MEH_TEXT_MOVING_AFTER;
	}

	if (text->restart_timestamp == -1 && SDL_GetTicks() > text->start_timestamp) {
		if (text->tex_w > c_text_w && (text->off_x + c_text_w) < text->tex_w) {
			text->off_x += 0.15f;
		}
		if (text->tex_h > c_text_h && (text->off_y + c_text_h) < text->tex_h) {
			text->off_y += 0.15f;
		}

		/* movement over, restart timestamp */
		if ((text->off_y + c_text_h) >= text->tex_h && (text->off_x + c_text_w) >= text->tex_w) {
			text->restart_timestamp = SDL_GetTicks() + MEH_TEXT_MOVING_AFTER*2;
		}
	}
}

void meh_widget_text_reset_move(WidgetText* text) {
	text->start_timestamp = -1;
	text->restart_timestamp = -1;
	text->off_x = 0;
	text->off_y = 0;
}

/*
 * meh_widget_text_render renders the given text.
 * NOTE that the internal x,y,w,h are in relative resolution (must be converted)
 * but the generated texture name are in the current display mode and
 * doesn't need to be converted
 */
void meh_widget_text_render(Window* window, WidgetText* text) {
	g_assert(text != NULL);
	g_assert(window != NULL);

	if (text->texture == NULL) {
		meh_widget_text_reload(window, text);
	}

	int c_text_w = meh_window_convert_width(window, text->w);
	int c_text_h = meh_window_convert_height(window, text->h);

	/* compute the src rect to use in the texture */
	int src_x, src_y, src_w, src_h;
	src_x = text->off_x;
	src_y = text->off_y;
	src_w = text->tex_w;
	src_h = text->tex_h;

	if (text->tex_w > c_text_w) {
		src_w = c_text_w;
	}
	if (text->tex_h > c_text_h) {
		src_h = c_text_h;
	}

	SDL_Rect src = {
		src_x,
		src_y,
		src_w,
		src_h
	};

	/* dst target */

	/* don't render to large */
	int dst_w = c_text_w;
	int dst_h = c_text_h;
	if (text->tex_w < c_text_w) {
		dst_w = text->tex_w;
	}
	if (text->tex_h < c_text_h) {
		dst_h = text->tex_h;
	}

	SDL_Rect dst = { 
		meh_window_convert_width(window, text->x.value),
		meh_window_convert_height(window, text->y.value),
		dst_w,
		dst_h
	};

	/* draw */
	meh_window_render_texture(window, text->texture, &src, &dst);
}
