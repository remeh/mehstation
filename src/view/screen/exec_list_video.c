#include "system/consts.h"
#include "view/screen.h"
#include "view/window.h"
#include "view/screen/exec_list_video.h"

#define EXEC_LIST_VIDEO_WIDTH 320
#define EXEC_LIST_VIDEO_HEIGHT 240

static gchar* find_video_filename(Executable* executable);

ExecListVideo* meh_exec_list_video_new(Window* window, Screen* screen, Executable* executable) {
	g_assert(window != NULL);

	gchar* filename = find_video_filename(executable);

	ExecListVideo* exec_list_video = g_new(ExecListVideo, 1);
	exec_list_video->video_widget = NULL;

	/* filename provided, create the widget video */
	if (filename != NULL && strlen(filename) > 0) {
		exec_list_video->video_widget = meh_widget_video_new(window, filename, 500, MEH_FAKE_HEIGHT+600, EXEC_LIST_VIDEO_WIDTH, EXEC_LIST_VIDEO_HEIGHT);

		/* if the widget video has been successfully created,
		 * animate the entrance of the widget. */
		if (exec_list_video->video_widget != NULL) {
			exec_list_video->video_widget->w_image->y = meh_transition_start(
								MEH_TRANSITION_CUBIC,
								exec_list_video->video_widget->w_image->y.value,
								350,
								300
							);
			meh_screen_add_image_transitions(screen, exec_list_video->video_widget->w_image);
		}
	}

	exec_list_video->executable = executable;

	return exec_list_video;
}

void meh_exec_list_video_destroy(ExecListVideo* exec_list_video) {
	if (exec_list_video == NULL) {
		return;
	}

	if (exec_list_video->video_widget != NULL) {
		meh_widget_video_destroy(exec_list_video->video_widget);
	}

	g_free(exec_list_video);
}

/* meh_exec_list_video_has_video returns whether or not the given ExecListVideo
 * will read and display a video. */
gboolean meh_exec_list_video_has_video(ExecListVideo* exec_list_video) {
	if (exec_list_video == NULL || exec_list_video->video_widget == NULL) {
		return FALSE;
	}
	return TRUE;
}

void meh_exec_list_video_update(Screen* screen, ExecListVideo* exec_list_video) {
	g_assert(screen != NULL);

	if (exec_list_video == NULL || exec_list_video->video_widget == NULL) {
		return;
	}

	meh_widget_video_update(exec_list_video->video_widget);
}

void meh_exec_list_video_render(Window* window, ExecListVideo* exec_list_video) {
	g_assert(window != NULL);

	if (exec_list_video == NULL || exec_list_video->video_widget == NULL) {
		return;
	}

	meh_widget_video_render(window, exec_list_video->video_widget);

	return;
}

static gchar* find_video_filename(Executable* executable) {
	if (executable == NULL) {
		return "";
	}

	for (unsigned int i = 0; i < g_queue_get_length(executable->resources); i++) {
		ExecutableResource* res = g_queue_peek_nth(executable->resources, i);
		if (res != NULL) {
			if (g_strcmp0(res->type, "video") == 0) {
				return res->filepath;
			}
		}
	}

	return "";
}
