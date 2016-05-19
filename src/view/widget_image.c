/*
 * mehstation - Image widget.
 *
 * NOTE(remy): to implements the feature "keep aspect ratio",
 *             a method called meh_widget_image_set_texture will
 *             be needed as some calls to meh_widget_image_new
 *             contains a NULL texture to allow to set the texture
 *             after the creation of the widget.
 *             It's on this set_texture call that the aspect ratio
 *             should be computed.
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

	meh_widget_image_set_texture(i, texture, TRUE); /* TODO(remy): turn diiis ON! */

	return i;
}

void meh_widget_image_set_texture(WidgetImage* image, SDL_Texture* texture, gboolean keep_ratio) {
	g_assert(image != NULL);

	image->texture = texture;

	image->offset_x = 0;
	image->offset_y = 0;
	image->display_w = image->w.value;
	image->display_h = image->h.value;

	if (texture != NULL && keep_ratio) {
		meh_widget_image_compute_aspect_ratio(image);
	}
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
		meh_window_convert_width(window, image->x.value + image->offset_x),
		meh_window_convert_height(window, image->y.value + image->offset_y),
		meh_window_convert_width(window, image->display_w), /* TODO must be transition? */
		meh_window_convert_height(window, image->display_h)
	};

	meh_window_render_texture(window, image->texture, NULL, &rect);
}

void meh_widget_image_compute_aspect_ratio(WidgetImage* image) {
	g_assert(image != NULL);

	if (image->texture == NULL) {
		g_critical("meh_widget_image_compute_aspect_ratio called with NULL texture.");
		return;
	}

	SDL_QueryTexture(image->texture, NULL, NULL, &image->tex_w, &image->tex_h);

	/* compute the size */

	image->display_w = image->tex_w;
	image->display_h = image->tex_h;

	if (image->tex_w > image->w.value) {
		image->display_w = image->w.value;
		image->display_h = (image->w.value / (float)image->tex_w) * (float)image->tex_h;
	}

	if (image->display_h > image->h.value) {
		image->display_h = image->h.value;
		image->display_w = (image->h.value / (float)image->tex_h) * (float)image->tex_w;
	}

	/* TODO(remy): what if h > tex_h ? */

	/* compute the position inside the target rect */

	image->offset_x = 0;
	if (image->w.value - image->display_w > 0.01) {
		image->offset_x = (image->w.value - image->display_w)/2;
	}

	g_debug("offset_x: %d", image->offset_x);

	image->offset_y = 0;
	if (image->h.value - image->display_h > 0.01) {
		image->offset_y = (image->h.value - image->display_h)/2;
	}

	g_debug("offset_y: %d", image->offset_y);
}
