/*
 * mehstation - Video to display in the exec list.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "system/db/models.h"
#include "view/screen.h"
#include "view/window.h"
#include "view/widget_video.h"

typedef struct ExecListVideo {
	/* do not free this executable memory */
	Executable* executable;
	WidgetVideo* video_widget;
} ExecListVideo;

ExecListVideo* meh_exec_list_video_new(Window* window, Screen* screen, Executable* executable);
void meh_exec_list_video_update(Screen* screen, ExecListVideo* exec_list_video);
void meh_exec_list_video_render(Window* window, ExecListVideo* exec_list_video);
gboolean meh_exec_list_video_has_video(ExecListVideo* exec_list_video);
void meh_exec_list_video_destroy();
