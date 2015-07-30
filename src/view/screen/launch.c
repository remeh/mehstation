/*
 * mehstation - Launch an executable.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include "system/app.h"
#include "system/consts.h"
#include "view/screen.h"
#include "view/widget_rect.h"
#include "view/screen/launch.h"

Screen* meh_screen_launch_new(App* app, Screen* src_screen, Platform* platform, Executable* executable,
															WidgetImage* src_widget) {
	g_assert(app != NULL);
	g_assert(src_screen != NULL);
	g_assert(platform != NULL);
	g_assert(executable != NULL);

	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Launch screen");
	screen->messages_handler = &meh_screen_launch_messages_handler;
	screen->destroy_data = &meh_screen_launch_destroy_data;

	/*
	 * custom data
	 */
	LaunchData* data = g_new(LaunchData, 1);

	data->src_screen = src_screen;
	data->image_widget = NULL;
	data->executable = executable;
	data->platform = platform;
	data->src_widget = src_widget;
	data->zoom_logo = FALSE;

	/* fading rect */
	SDL_Color black = { 0, 0, 0 ,0 };
	data->fade_widget = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, black, TRUE);
	data->fade_widget->a = meh_transition_start(MEH_TRANSITION_LINEAR, 1, 254, app->settings.fade_duration*4);
	meh_screen_add_rect_transitions(screen, data->fade_widget);

	if (app->settings.zoom_logo && src_widget != NULL) {
		/* cover image */
		data->image_widget = meh_widget_image_new(
									src_widget->texture,
									src_widget->x.value,
									src_widget->y.value,
									src_widget->h.value,
									src_widget->w.value);

		/* starts the cover transitions */
		data->image_widget->x = meh_transition_start(MEH_TRANSITION_CUBIC, src_widget->x.value, -(MEH_FAKE_WIDTH), app->settings.fade_duration*4);
		data->image_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, src_widget->y.value, -(MEH_FAKE_HEIGHT), app->settings.fade_duration*4);
		data->image_widget->w = meh_transition_start(MEH_TRANSITION_CUBIC, src_widget->w.value, MEH_FAKE_WIDTH*3, app->settings.fade_duration*4);
		data->image_widget->h = meh_transition_start(MEH_TRANSITION_CUBIC, src_widget->h.value, MEH_FAKE_HEIGHT*3, app->settings.fade_duration*4);
		meh_screen_add_image_transitions(screen, data->image_widget);

		data->zoom_logo = TRUE;
	}

	screen->data = data;

	return screen;
}

/*
 * meh_screen_launch_destroy_data destroys the additional data
 * of the launch screen.
 */
void meh_screen_launch_destroy_data(Screen* screen) {
	LaunchData* data = meh_screen_launch_get_data(screen);
	meh_widget_rect_destroy(data->fade_widget);
	if (data->zoom_logo) {
		/* we delete the image widget we own, but not the one from the exec list */
		meh_widget_image_destroy(data->image_widget);
	}
	screen->data = NULL;
}

LaunchData* meh_screen_launch_get_data(Screen* screen) {
	g_assert(screen != NULL);
	if (screen->data == NULL) {
		return NULL;
	}
	return (LaunchData*) screen->data;
}

/*
 * meh_screen_launch_update udpates the content of the launch screen.
 */
int meh_screen_launch_update(struct App* app, Screen* screen) {
	/* Animate the fading rect. */
	meh_screen_update_transitions(screen);

	LaunchData* data = meh_screen_launch_get_data(screen);
	g_assert(data != NULL);

	/* update the src screen */
	meh_message_send(app, data->src_screen, MEH_MSG_UPDATE, NULL);
		
	if (data->fade_widget->a.ended == TRUE) {
		/* it's time to start the executable and to switch back to the src screen */
		meh_app_start_executable(app, data->platform, data->executable);
		meh_app_set_current_screen(app, data->src_screen, TRUE);
		meh_screen_destroy(screen); /* destroy the launch screen */
	}

	return 0;
}

int meh_screen_launch_messages_handler(struct App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 1;
	}

	switch (message->id) {
		case MEH_MSG_UPDATE:
			{
				meh_screen_launch_update(app, screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				meh_screen_launch_render(app, screen);
			}
			break;
	}

	return 0;
}

void meh_screen_launch_render(struct App* app, Screen* screen) {
	LaunchData* data = meh_screen_launch_get_data(screen);
	g_assert(data != NULL);

	gboolean* flip = g_new(gboolean, 1);
	*flip = FALSE;
	/* render the dest screen behind the fade widget */
	meh_message_send(app, data->src_screen, MEH_MSG_RENDER, flip);

	if (data->zoom_logo == TRUE) {
		/* render the cover */
		meh_widget_image_render(app->window, data->image_widget);
	}

	/* render the fade widget */
	meh_widget_rect_render(app->window, data->fade_widget);


	meh_window_render(app->window);
}

