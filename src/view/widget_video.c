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
	/* create the embedded image widget */
	WidgetImage* i = meh_widget_image_new(NULL, x, y, w, h);

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

