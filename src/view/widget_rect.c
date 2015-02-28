/*
 * mehstation - Rect widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <SDL2/SDL.h>
#include <glib.h>

#include "view/widget_rect.h"

/*
 * meh_widget_rect_destroy allocates a new widget rect.
 */
WidgetRect* meh_widget_rect_new(int x, int y, int w, int h, SDL_Color color, gboolean filled) {
	WidgetRect* r = g_new(WidgetRect, 1);

	r->x = meh_transition_start(MEH_TRANSITION_NONE, x, x, 0);
	meh_transition_end(&r->x);
	r->y = meh_transition_start(MEH_TRANSITION_NONE, y, y, 0);
	meh_transition_end(&r->y);
	r->w = meh_transition_start(MEH_TRANSITION_NONE, w, w, 0);
	meh_transition_end(&r->w);
	r->h = meh_transition_start(MEH_TRANSITION_NONE, h, h, 0);
	meh_transition_end(&r->h);

	r->r = meh_transition_start(MEH_TRANSITION_NONE, color.r, color.r, 0);
	meh_transition_end(&r->r);
	r->g = meh_transition_start(MEH_TRANSITION_NONE, color.g, color.g, 0);
	meh_transition_end(&r->g);
	r->b = meh_transition_start(MEH_TRANSITION_NONE, color.b, color.b, 0);
	meh_transition_end(&r->b);
	r->a = meh_transition_start(MEH_TRANSITION_NONE, color.a, color.a, 0);
	meh_transition_end(&r->a);

	r->filled = filled;

	return r;
}

/*
 * meh_widget_rect_destroy frees the resource of the given widget.
 */
void meh_widget_rect_destroy(WidgetRect* rect) {
	g_free(rect);
}

void meh_widget_rect_render(Window* window, const WidgetRect* rect) {
	g_assert(rect != NULL);
	g_assert(window != NULL);

	SDL_SetRenderDrawColor(window->sdl_renderer, rect->r.value, rect->g.value, rect->b.value, rect->a.value);

	if (rect->a.value != SDL_ALPHA_OPAQUE) {
		SDL_SetRenderDrawBlendMode(window->sdl_renderer, SDL_BLENDMODE_BLEND);
	} else {
		SDL_SetRenderDrawBlendMode(window->sdl_renderer, SDL_BLENDMODE_NONE);

	}

	SDL_Rect sdl_rect = { rect->x.value, rect->y.value, rect->w.value, rect->h.value };

	if (rect->filled) {
		SDL_RenderFillRect(window->sdl_renderer, &sdl_rect);
	} else {
		SDL_RenderDrawRect(window->sdl_renderer, &sdl_rect);
	}
}

