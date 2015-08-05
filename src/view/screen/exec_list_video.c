#include "system/consts.h"
#include "view/screen.h"
#include "view/window.h"
#include "view/screen/exec_list_video.h"

#define EXEC_LIST_VIDEO_WIDTH 320
#define EXEC_LIST_VIDEO_HEIGHT 240

// FIXME(remy): there is a memory bug (!!) if this delay is really slow.
#define EXEC_LIST_VIDEO_DELAY 800

static gchar* find_video_filename(Executable* executable);

ExecListVideo* meh_exec_list_video_new(Window* window, Screen* screen, Executable* executable) {
	g_assert(window != NULL);

	gchar* filename = find_video_filename(executable);

	ExecListVideo* exec_list_video = g_new(ExecListVideo, 1);
	exec_list_video->video_widget = NULL;
	exec_list_video->animation_started = FALSE;
	exec_list_video->start_after = 0;

	/* filename provided, create the widget video */
	if (filename != NULL && strlen(filename) > 0) {
		exec_list_video->video_widget = meh_widget_video_new(window, filename, -600, (MEH_FAKE_HEIGHT/2) - (EXEC_LIST_VIDEO_HEIGHT/2), EXEC_LIST_VIDEO_WIDTH, EXEC_LIST_VIDEO_HEIGHT);
	}

	SDL_Color black = { 0, 0, 0, 230 };

	exec_list_video->bg_widget = meh_widget_rect_new(100, -MEH_FAKE_HEIGHT, 400, MEH_FAKE_HEIGHT, black, TRUE);
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

	meh_widget_rect_destroy(exec_list_video->bg_widget);

	g_free(exec_list_video);
}

void meh_exec_list_video_update(Screen* screen, ExecListVideo* exec_list_video) {
	g_assert(screen != NULL);

	if (exec_list_video == NULL || exec_list_video->video_widget == NULL) {
		return;
	}

	/* at first update, start the delay */
	if (exec_list_video->start_after == 0) {
		exec_list_video->start_after = SDL_GetTicks()+EXEC_LIST_VIDEO_DELAY;
	}

	if (SDL_GetTicks() > exec_list_video->start_after) {
		if (!exec_list_video->animation_started) {
			exec_list_video->bg_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, exec_list_video->bg_widget->y.value, 0, 500);
			meh_screen_add_rect_transitions(screen, exec_list_video->bg_widget);

			exec_list_video->video_widget->w_image->x = meh_transition_start(
								MEH_TRANSITION_CUBIC,
								exec_list_video->video_widget->w_image->x.value,
								138, 
								500
							);
			meh_screen_add_image_transitions(screen, exec_list_video->video_widget->w_image);
			exec_list_video->animation_started = TRUE;
		}

		meh_widget_video_update(exec_list_video->video_widget);
	}
}

void meh_exec_list_video_render(Window* window, ExecListVideo* exec_list_video) {
	g_assert(window != NULL);

	if (exec_list_video == NULL || exec_list_video->video_widget == NULL) {
		return;
	}

	/* do not render if nothing is on the screen */
	if (SDL_GetTicks() > exec_list_video->start_after) {
		meh_widget_rect_render(window, exec_list_video->bg_widget);
		meh_widget_video_render(window, exec_list_video->video_widget);
	}

	return;
}

static gchar* find_video_filename(Executable* executable) {
	return "/var/tmp/video.mp4";
}
