/*
 * mehstation - Video widget.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "view/widget_video.h"

static void destroy_internal_video(WidgetVideo* w_video);

/*
 * meh_widget_video_new allocates a new widget video.
 * and prepares its internal texture.
 */
WidgetVideo* meh_widget_video_new(Window* window, gchar* filename, float x, float y, float w, float h) {
	/* create the embedded image widget */
	WidgetImage* i = meh_widget_image_new(NULL, x, y, w, h);

	WidgetVideo* v = g_new(WidgetVideo, 1);

	v->video = NULL;
	v->w_image = i;

	meh_widget_video_set(window, v, filename);

	return v;
}

static void destroy_internal_video(WidgetVideo* w_video) {
	if (w_video->video != NULL) {
		meh_video_destroy(w_video->video);
		w_video->video = NULL;
		w_video->w_image->texture = NULL;
	}
}

void meh_widget_video_set(Window* window, WidgetVideo* w_video, gchar* filename) {
	if (w_video == NULL) {
		return;
	}

	/* clear the memory if necessary */
	destroy_internal_video(w_video);

	if (filename != NULL || strlen(filename) > 0) {
		w_video->video = meh_video_new(window, filename);

		if (w_video->video == NULL) {
			g_critical("Can't create the WidgetVideo for the video '%s'", filename);
		}

		/* assign the video texture to the internal WidgetImage texture */
		if (w_video->video != NULL) {
			w_video->w_image->texture = w_video->video->texture;
		}	
	}
}

/*
 * meh_widget_video_destroy frees the resource of the given widget.
 */
void meh_widget_video_destroy(WidgetVideo* w_video) {
	if (w_video == NULL) {
		return;
	}

	destroy_internal_video(w_video);

	w_video->w_image->texture = NULL;
	meh_widget_image_destroy(w_video->w_image);
	g_free(w_video);
}

void meh_widget_video_update(WidgetVideo* w_video) {
	if (w_video == NULL || w_video->video == NULL) {
		return;
	}

	meh_video_update(w_video->video);
}

/*
 * meh_widget_video_render renders the given video onto the given window.
 */
void meh_widget_video_render(Window* window, WidgetVideo* w_video) {
	g_assert(window != NULL);

	if (w_video == NULL || w_video->video == NULL) {
		return;
	}

	meh_widget_image_render(window, w_video->w_image);
}

