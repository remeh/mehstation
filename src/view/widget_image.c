/*
 * mehstation - Image widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "view/widget_image.h"

/*
 * meh_widget_image_destroy allocates a new widget image.
 */
WidgetImage* meh_widget_image_new(SDL_Texture* texture, int x, int y, int w, int h) {
	WidgetImage* i = g_new(WidgetImage, 1);

	i->x = meh_transition_start(MEH_TRANSITION_NONE, x, x, 0);
	meh_transition_end(&i->x);
	i->y = meh_transition_start(MEH_TRANSITION_NONE, y, y, 0);
	meh_transition_end(&i->y);

	i->w = meh_transition_start(MEH_TRANSITION_NONE, w, w, 0);
	meh_transition_end(&i->w);
	i->h = meh_transition_start(MEH_TRANSITION_NONE, h, h, 0);
	meh_transition_end(&i->h);

	i->texture = texture;

	return i;
}

/*
 * meh_widget_image_destroy frees the resource of the given widget.
 */
void meh_widget_image_destroy(WidgetImage* image) {
	g_free(image);
}

void meh_widget_image_render(Window* window, const WidgetImage* image) {
	g_assert(image != NULL);
	g_assert(window != NULL);

	if (image->texture == NULL) {
		return;
	}

	SDL_Rect rect = { image->x.value, image->y.value, image->w.value, image->h.value };
	meh_window_render_texture(window, image->texture, rect);
}
