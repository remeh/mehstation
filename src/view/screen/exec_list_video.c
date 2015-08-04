#include "system/consts.h"
#include "view/screen.h"
#include "view/window.h"
#include "view/screen/exec_list_video.h"

#define EXEC_LIST_VIDEO_WIDTH 320
#define EXEC_LIST_VIDEO_HEIGHT 240

static gchar* find_video_filename(Executable* executable);

ExecListVideo* meh_exec_list_video_new(Window* window, Executable* executable) {
	g_assert(window != NULL);
	g_assert(executable != NULL);

	gchar* filename = find_video_filename(executable);

	ExecListVideo* exec_list_video = g_new(ExecListVideo, 1);
	exec_list_video->video_widget = NULL;

	/* filename provided, create the widget video */
	if (filename != NULL && strlen(filename) > 0) {
		exec_list_video->video_widget = meh_widget_video_new(window, filename, MEH_FAKE_WIDTH-600, (MEH_FAKE_HEIGHT/2) - (EXEC_LIST_VIDEO_HEIGHT/2), EXEC_LIST_VIDEO_WIDTH, EXEC_LIST_VIDEO_HEIGHT);
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
	return "/var/tmp/video.mp4";
}
