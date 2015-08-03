/*
 * mehstation - Video widget.
 * A video widget is an embedded image widget
 * having an update method.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include <SDL2/SDL.h>
#include <glib.h>

#include "view/image.h"
#include "view/video.h"
#include "view/window.h"
#include "view/widget_image.h"
#include "system/transition.h"

typedef struct {
	WidgetImage* w_image;

	Video* video;
} WidgetVideo;

WidgetVideo* meh_widget_video_new(Window* window, gchar* filename, float x, float y, float w, float h);
void meh_widget_video_update(const WidgetVideo* video);
void meh_widget_video_destroy(WidgetVideo* video);
void meh_widget_video_render(Window* window, const WidgetVideo* video);

