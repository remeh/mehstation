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
WidgetImage* meh_widget_image_new(SDL_Texture* texture, float x, float y, float w, float h) {
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

	SDL_QueryTexture(texture, NULL, NULL, &i->tex_w, &i->tex_h);

	return i;
}

/*
 * meh_widget_image_destroy frees the resource of the given widget.
 */
void meh_widget_image_destroy(WidgetImage* image) {
	image->texture = NULL;
	g_free(image);
}

void meh_widget_image_render(Window* window, const WidgetImage* image) {
	g_assert(image != NULL);
	g_assert(window != NULL);

	if (image->texture == NULL) {
		return;
	}

	// Convert the normalized position to the window position.
	SDL_Rect rect = {
		meh_window_convert_width(window, image->x.value),
		meh_window_convert_height(window, image->y.value),
		meh_window_convert_width(window, image->w.value),
		meh_window_convert_height(window, image->h.value)
	};

	meh_window_render_texture(window, image->texture, NULL, &rect);
}
