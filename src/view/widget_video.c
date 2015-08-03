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
WidgetVideo* meh_widget_video_new(Window* window, gchar* filename, float x, float y, float w, float h) {
	WidgetImage* i = g_new(WidgetImage, 1);

	i->x = meh_transition_start(MEH_TRANSITION_NONE, x, x, 0);
	meh_transition_end(&i->x);
	i->y = meh_transition_start(MEH_TRANSITION_NONE, y, y, 0);
	meh_transition_end(&i->y);

	i->w = meh_transition_start(MEH_TRANSITION_NONE, w, w, 0);
	meh_transition_end(&i->w);
	i->h = meh_transition_start(MEH_TRANSITION_NONE, h, h, 0);
	meh_transition_end(&i->h);

	WidgetVideo* v = g_new(WidgetVideo, 1);
	v->w_image = i;

	v->video = meh_video_new(window, filename);
	if (v->video == NULL) {
		g_critical("Can't create the WidgetVideo for the video '%s'", filename);
	}

	/* assign the video texture to the
	 * internal WidgetImage texture */
	v->w_image->texture = v->video->texture;

	return v;
}

/*
 * meh_widget_video_destroy frees the resource of the given widget.
 */
void meh_widget_video_destroy(WidgetVideo* w_video) {
	if (w_video->video != NULL) {
		meh_video_destroy(w_video->video);
	}
	w_video->w_image->texture = NULL;
	meh_widget_image_destroy(w_video->w_image);
	g_free(w_video);
}

void meh_widget_video_update(const WidgetVideo* w_video) {
	meh_video_update(w_video->video);
}

/*
 * meh_widget_video_render renders the given video onto the given window.
 */
void meh_widget_video_render(Window* window, const WidgetVideo* video) {
	g_assert(video != NULL);
	g_assert(window != NULL);

	meh_widget_image_render(window, video->w_image);
}

