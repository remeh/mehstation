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
#include "view/window.h"
#include "view/widget_image.h"
#include "system/transition.h"

typedef struct {
	WidgetImage* w_image;
} WidgetVideo;

WidgetVideo* meh_widget_video_new(float x, float y, float w, float h);
void meh_widget_video_destroy(WidgetVideo* video);
void meh_widget_video_render(Window* window, const WidgetVideo* video);

