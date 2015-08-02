/*
 * mehstation - Video widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "view/widget_video.h"

/*
 * meh_widget_video_new allocates a new widget video.
 * and prepares its internal texture.
 */
WidgetVideo* meh_widget_video_new(float x, float y, float w, float h) {
	WidgetImage* i = g_new(WidgetImage, 1);

	i->x = meh_transition_start(MEH_TRANSITION_NONE, x, x, 0);
	meh_transition_end(&i->x);
	i->y = meh_transition_start(MEH_TRANSITION_NONE, y, y, 0);
	meh_transition_end(&i->y);

	i->w = meh_transition_start(MEH_TRANSITION_NONE, w, w, 0);
	meh_transition_end(&i->w);
	i->h = meh_transition_start(MEH_TRANSITION_NONE, h, h, 0);
	meh_transition_end(&i->h);

	// TODO(remy): prepare the internal texture.

	WidgetVideo* v = g_new(WidgetVideo, 1);
	v->w_image = i;

	return v;
}

/*
 * meh_widget_video_destroy frees the resource of the given widget.
 */
void meh_widget_video_destroy(WidgetVideo* video) {
	// TODO(remy): free the resources of the texture
	video->w_image->texture = NULL;
	meh_widget_image_destroy(video->w_image);
	g_free(video);
}

// TODO(remy): create update method.

/*
 * meh_widget_video_render renders the given video onto the given window.
 */
void meh_widget_video_render(Window* window, const WidgetVideo* video) {
	g_assert(video != NULL);
	g_assert(window != NULL);

	if (video->w_image->texture == NULL) {
		return;
	}

	// Convert the normalized position to the window position.
	SDL_Rect rect = {
		meh_window_convert_width(window, video->w_image->x.value),
		meh_window_convert_height(window, video->w_image->y.value),
		meh_window_convert_width(window, video->w_image->w.value),
		meh_window_convert_height(window, video->w_image->h.value)
	};

	meh_window_render_texture(window, video->w_image->texture, NULL, &rect);
}

