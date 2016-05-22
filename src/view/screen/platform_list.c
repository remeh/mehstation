/*
 * mehstation - Platform list screen.
 *
 * TODO(remy):
 * - refactor the platform/extra part
 *   to use the same indexes, queues, etc.
 *   Should use a struct looking similar
 *   to the current lastPlayed one, probably
 *   called menuEntry or something else.
 * - the start executable code is a bit
 *   duplicated with other screens/popups.
 *
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include <glib.h>
#include <string.h>

#include "system/app.h"
#include "system/consts.h"
#include "system/input.h"
#include "system/message.h"
#include "system/transition.h"
#include "system/utils.h"
#include "system/db/models.h"
#include "view/screen.h"
#include "view/widget_text.h"
#include "view/screen/executable_list.h"
#include "view/screen/fade.h"
#include "view/screen/platform_list.h"
#include "view/screen/simple_popup.h"
#include "view/screen/main_popup.h"

static Platform* meh_screen_platform_get_current(Screen* screen);
static void meh_screen_platform_last_started_load(App* app, Screen* screen);
static void meh_screen_platform_last_started_destroy(Screen* screen);
static void meh_screen_platform_start_executable(App* app, Screen* screen);

Screen* meh_screen_platform_list_new(App* app) {
	Screen* screen = meh_screen_new(app->window);

	screen->name = g_strdup("Platform list screen");
	screen->messages_handler = &meh_screen_platform_list_messages_handler;
	screen->destroy_data = &meh_screen_platform_list_destroy_data;

	/* init the screen data. */
	PlatformListData* data = g_new(PlatformListData, 1);	
	data->selected_platform = 0;

	data->background = NULL;
	data->background_widget = meh_widget_image_new(NULL, 0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT);

	/*
	 * Widgets
	 */

	/* Title */
	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color black = { 0, 0, 0, 210 };
	SDL_Color gray = { 10, 10, 10, 0 };
	data->title = meh_widget_text_new(app->big_font, app->settings.name, 50, 50, 1230, 50, white, FALSE);
	data->title->x = meh_transition_start(MEH_TRANSITION_CUBIC, -200, 50, 1000);
	meh_screen_add_text_transitions(screen, data->title);

	/* No platforms text */
	data->no_platforms_widget = meh_widget_text_new(app->big_font, "No platforms configured. Use mehstation-config", 150, 330, MEH_FAKE_WIDTH-150, 50, white, FALSE);

	/* Platforms */
	data->platforms = meh_db_get_platforms(app->db);
	data->icons_widgets = g_queue_new();
	data->platforms_icons = g_queue_new();

	/* Load the data / icons / widgets of every platforms */
	for (unsigned int i = 0; i < g_queue_get_length(data->platforms); i++) {
		Platform* platform = g_queue_peek_nth(data->platforms, i);

		/* load the platform icon */
		SDL_Texture* p_texture = NULL;
		if (platform->icon == NULL || strlen(platform->icon) == 0) {
			/* create a texture with just the text of the platform */
			p_texture = meh_font_render_on_texture(
							app->window->sdl_renderer,
							app->small_font,
							platform->name,
							white,
							TRUE
						);
		} else {
			/* load the icon */
			p_texture = meh_image_load_file(app->window->sdl_renderer, platform->icon);
		}

		if (p_texture == NULL) {
			g_critical("Can't load the icon of the platform %s" ,platform->name);
		}

		/* store the texture */
		g_queue_push_tail(data->platforms_icons, p_texture);

		/* create the platform widget */
		WidgetImage* platform_widget = meh_widget_image_new(p_texture, 100, 285 + (i*200), 150, 150);
		g_queue_push_tail(data->icons_widgets, platform_widget);
	}

	/* background hovers */
	data->background_hover = meh_widget_rect_new(0, 0, MEH_FAKE_WIDTH, MEH_FAKE_HEIGHT, gray, TRUE);
	data->hover = meh_widget_rect_new(0, 260, MEH_FAKE_WIDTH, 200, black, TRUE);

	/* misc */
	data->maintext = meh_widget_text_new(app->big_font, "", 320, 315, 500, 100, white, FALSE);
	data->maintext->x = meh_transition_start(MEH_TRANSITION_CUBIC, MEH_FAKE_WIDTH+200, 320, 300);
	meh_screen_add_text_transitions(screen, data->maintext);
	data->subtext = meh_widget_text_new(app->small_font, "", 325, 365, 500, 100, white, FALSE);
	data->subtext->x = meh_transition_start(MEH_TRANSITION_CUBIC, MEH_FAKE_WIDTH+200, 360, 300);
	meh_screen_add_text_transitions(screen, data->subtext);

	screen->data = data;

	/* last played */
	data->last_started.executable = NULL;
	data->last_started.platform = NULL;
	data->last_started.widget_icon = NULL;
	data->last_started.icon = NULL;
	meh_screen_platform_last_started_load(app, screen);

	/* go to the platform 0 */
	meh_screen_platform_change_platform(app, screen);

	return screen;
}

static void meh_screen_platform_last_started_load(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	if (data == NULL) {
		return;
	}

	/* find last started executable */
	int platform_id = -1;
	Executable* executable = meh_db_get_last_started_executable(app->db, &platform_id);

	/* didn't find any */
	if (platform_id < 0 || executable == NULL) {
		g_critical("Can't find the last played executable.");
		return;
	}

	/* platform */
	Platform* platform = meh_db_get_platform(app->db, platform_id);
	if (platform == NULL) {
		g_critical("Can't find the platform %d to load the last played executable.", platform_id);
		meh_model_executable_destroy(executable);
		return;
	}

	/* icon */
	SDL_Texture* icon = NULL;
	gchar* cover_filepath = meh_db_get_executable_resource_path(app->db, executable, MEH_EXEC_RES_COVER);
	if (cover_filepath != NULL) {
		icon = meh_image_load_file(app->window->sdl_renderer, cover_filepath);
		g_free(cover_filepath); cover_filepath = NULL;
	} else {
		/* no filepath, fallback on the platform icon */
		// TODO(remy): we should use a placeholder image instead,
		//             because the platform don't necessarily have
		//             an icon.
		icon = meh_image_load_file(app->window->sdl_renderer, platform->icon);
	}

	if (icon == NULL) {
		g_critical("Can't load the file for the last played executable.");
		meh_model_executable_destroy(executable);
		meh_model_platform_destroy(platform);
		return;
	}

	/* widget */
	WidgetImage* widget_icon = meh_widget_image_new(icon, 100, 285 - 200, 150, 150);

	data->last_started.widget_icon = widget_icon;
	data->last_started.platform = platform;
	data->last_started.executable = executable;
	data->last_started.icon = icon;

	g_debug("Last played executable loaded: %s", executable->display_name);
}

static void meh_screen_platform_last_started_destroy(Screen* screen) {
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	if (data == NULL) {
		return;
	}

	/* last played */
	if (data->last_started.executable != NULL) {
		meh_model_executable_destroy(data->last_started.executable);
		data->last_started.executable = NULL;
	}
	if (data->last_started.platform != NULL) {
		meh_model_platform_destroy(data->last_started.platform);
		data->last_started.platform = NULL;
	}
	if (data->last_started.icon != NULL) {
		SDL_DestroyTexture(data->last_started.icon);
		data->last_started.icon = NULL;
	}
	if (data->last_started.widget_icon != NULL) {
		meh_widget_image_destroy(data->last_started.widget_icon);
		data->last_started.widget_icon = NULL;
	}
}

/*
 * meh_screen_platform_list_destroy_data role is to delete the typed data of the screen
 */
void meh_screen_platform_list_destroy_data(Screen* screen) {
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	if (data == NULL) {
		return;
	}

	/* free platforms icons texture */
	for (unsigned int i = 0; i < g_queue_get_length(data->platforms_icons); i++) {
		SDL_Texture* text = g_queue_peek_nth(data->platforms_icons, i);
		SDL_DestroyTexture(text);
	}
	g_queue_free(data->platforms_icons);

	/* free platforms widget */
	for (unsigned int i = 0; i < g_queue_get_length(data->icons_widgets); i++) {
		WidgetImage* widget = g_queue_peek_nth(data->icons_widgets, i);
		meh_widget_image_destroy(widget);
	}
	g_queue_free(data->icons_widgets);

	/* last played */
	meh_screen_platform_last_started_destroy(screen);

	/* free platform models */
	meh_model_platforms_destroy(data->platforms);

	/* various widgets */
	meh_widget_text_destroy(data->maintext);
	meh_widget_text_destroy(data->subtext);
	meh_widget_text_destroy(data->title);
	meh_widget_text_destroy(data->no_platforms_widget);

	/* background */
	if (data->background != NULL) {
		SDL_DestroyTexture(data->background);
	}
	meh_widget_image_destroy(data->background_widget);

	meh_widget_rect_destroy(data->background_hover);
	meh_widget_rect_destroy(data->hover);
}

/*
 * meh_screen_platform_list_get_data returns the data of the platform_list screen
 */
PlatformListData* meh_screen_platform_list_get_data(Screen* screen) {
	PlatformListData* data = (PlatformListData*) screen->data;
	return data;
}

static Platform* meh_screen_platform_get_current(Screen* screen) {
	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	Platform* platform = g_queue_peek_nth(data->platforms, data->selected_platform);
	return platform;
}

int meh_screen_platform_list_messages_handler(App* app, Screen* screen, Message* message) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	if (message == NULL) {
		return 0;
	}


	switch (message->id) {
		case MEH_MSG_BUTTON_PRESSED:
			{
				InputMessageData* data = (InputMessageData*)message->data;
				meh_screen_platform_list_button_pressed(app, screen, data->button);
			}
			break;
		case MEH_MSG_UPDATE:
			{
				meh_screen_platform_list_update(screen);
			}
			break;
		case MEH_MSG_RENDER:
			{
				if (message->data == NULL) {
					meh_screen_platform_list_render(app, screen, TRUE);
				} else {
					gboolean* flip = (gboolean*)message->data;
					meh_screen_platform_list_render(app, screen, *flip);
				}
			}
			break;
	}

	return 0;
}

static void meh_screen_platform_list_start_popup(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	Screen* popup_screen = meh_simple_popup_new(app, screen, NULL, NULL);
	meh_simple_popup_add_action(
			app,
			popup_screen,
			"Run random executable",
			&meh_main_popup_random_executable
			);
	meh_simple_popup_add_action(
			app,
			popup_screen,
			"Close mehstation",
			&meh_main_popup_close
			);
	meh_simple_popup_add_action(
			app,
			popup_screen,
			"Shutdown",
			&meh_main_popup_os_shutdown
			);
	meh_app_set_current_screen(app, popup_screen, TRUE);
	/* NOTE we don't free the memory of the current screen, the popup screen
	 * will go back to it later. */
}

static void meh_screen_platform_list_start_platform(App* app, Screen* screen) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	Platform* platform = meh_screen_platform_get_current(screen);

	if (platform != NULL) {
		/* create the child screen */
		Screen* exec_list_screen = meh_exec_list_new(app, platform->id);
		exec_list_screen->parent_screen = screen;
		Screen* fade_screen = meh_screen_fade_new(app, screen, exec_list_screen);
		meh_app_set_current_screen(app, fade_screen, TRUE);
		/* NOTE we don't free the memory of the starting screen, the fade screen
		 * will do it. */
	}
}

/*
 * meh_screen_platform_start_executable destroys the current
 * platform list screen after having saved some needed resources,
 * starts the executable and then re-creates a platform list
 * screen.
 */
static void meh_screen_platform_start_executable(App* app, Screen* screen) {
	PlatformListData* platform_list_data = meh_screen_platform_list_get_data(screen);

	/* take the ownership of the executable and the platform
	 * and recopy the selected platform to reselect */
	Executable* executable = platform_list_data->last_started.executable;
	platform_list_data->last_started.executable = NULL;
	Platform* platform = platform_list_data->last_started.platform;
	platform_list_data->last_started.platform = NULL;
	int selected_platform = platform_list_data->selected_platform;

	meh_screen_destroy(screen);

	meh_app_start_executable(app, platform, executable);

	meh_model_executable_destroy(executable);
	meh_model_platform_destroy(platform);

	/* recreate the platform screen */
	screen = meh_screen_platform_list_new(app);
	platform_list_data = meh_screen_platform_list_get_data(screen);
	platform_list_data->selected_platform = selected_platform;

	meh_screen_platform_change_platform(app, screen);

	meh_app_set_current_screen(app, screen, FALSE);
}

/*
 * meh_screen_platform_list_button_pressed is called when we received a button pressed
 * message.
 */
void meh_screen_platform_list_button_pressed(App* app, Screen* screen, int pressed_button) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);

	/* no platforms, ignore keystrokes and stop the
	 * main loop on escape */
	if (g_queue_get_length(data->platforms) == 0) {
		if (pressed_button == MEH_INPUT_SPECIAL_ESCAPE) {
			app->mainloop.running = FALSE;
		}
		return;
	}

	switch (pressed_button) {
		case MEH_INPUT_SPECIAL_ESCAPE:
		case MEH_INPUT_BUTTON_START:
			meh_screen_platform_list_start_popup(app, screen);
			break;
		case MEH_INPUT_BUTTON_A:
			meh_audio_play(app->audio, SFX_BIP);
			if (data->selected_platform >= 0) {
				/* enter the platform */
				meh_screen_platform_list_start_platform(app, screen);
			} else if (data->selected_platform == -1) {
				/* last played */
				meh_screen_platform_start_executable(app, screen);
				return;
			}
			break;
		case MEH_INPUT_BUTTON_B:
			/* Switch the current_screen to the parent screen if any */
			if (screen->parent_screen != NULL) {
				meh_app_set_current_screen(app, screen->parent_screen, TRUE);
				/* this one won't be used anymore. */
				meh_screen_destroy(screen);
			}
			break;
		case MEH_INPUT_BUTTON_UP:
			/* last started entry or loop to the bottom of the list */
			if (data->selected_platform == -1 ||
				(data->last_started.executable == NULL && data->selected_platform == 0)) {
				data->selected_platform = g_queue_get_length(meh_screen_platform_list_get_data(screen)->platforms)-1;
			} else {
				data->selected_platform -= 1;
			}
			meh_audio_play(app->audio, SFX_BIP);
			meh_screen_platform_change_platform(app, screen);
			break;
		case MEH_INPUT_BUTTON_DOWN:
			/* go to either the last started entry if any
			 * or directly to the first available platform */
			if (data->selected_platform == g_queue_get_length(meh_screen_platform_list_get_data(screen)->platforms)-1) {
				if (data->last_started.executable != NULL) {
					data->selected_platform = -1;
				} else {
					data->selected_platform = 0;
				}
			} else {
				data->selected_platform += 1;
			}
			meh_audio_play(app->audio, SFX_BIP);
			meh_screen_platform_change_platform(app, screen);
			break;
	}
}

void meh_screen_platform_change_platform(App* app, Screen* screen) {
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);

	if (g_queue_get_length(data->platforms) == 0) {
		return;
	}

	Platform* platform = g_queue_peek_nth(data->platforms, data->selected_platform);

	/* destroys the background */
	if (data->background != NULL) {
		SDL_DestroyTexture(data->background);
		data->background = NULL;
	}

	/* icons */
	for (unsigned int i = 0; i < g_queue_get_length(data->icons_widgets); i++) {
		int y = 285;
		if (i < data->selected_platform) {
			y += (i - data->selected_platform) * 200;
		} else if (i > data->selected_platform) {
			y -= (data->selected_platform - i) * 200;
		}

		WidgetImage* image = g_queue_peek_nth(data->icons_widgets, i);
		image->y = meh_transition_start(MEH_TRANSITION_CUBIC, image->y.value, y, 200);
		meh_screen_add_image_transitions(screen, image);
	}

	/* extra icons */
	if (data->last_started.widget_icon != NULL) {
		int new_y = 85 - (data->selected_platform) * 200;
		WidgetImage* image = data->last_started.widget_icon;
		image->y = meh_transition_start(MEH_TRANSITION_CUBIC, image->y.value, new_y, 200);
		meh_screen_add_image_transitions(screen, image);
	}

	/* clear the previous text */
	g_free(data->maintext->text);
	g_free(data->subtext->text);

	/* animate a fade on the background */
	data->background_hover->a = meh_transition_start(MEH_TRANSITION_CUBIC, 255, 0, 300);
	meh_screen_add_rect_transitions(screen, data->background_hover);

	if (platform != NULL) {
		/* platform name */
		data->maintext->text = g_strdup(platform->name);

		/* executables count */
		int count_exec = meh_db_count_platform_executables(app->db, platform);
		data->subtext->text = g_strdup_printf("%d executable%s", count_exec, count_exec > 1 ? "s": "");

		/* background image */
		if (platform->background != NULL && strlen(platform->background) != 0) {
			data->background = meh_image_load_file(app->window->sdl_renderer, platform->background);
			meh_widget_image_set_texture(data->background_widget, data->background, FALSE);
		}
	} else if (data->selected_platform == -1 &&
			data->last_started.executable != NULL && data->last_started.platform != NULL) {
		Executable* executable = data->last_started.executable;
		data->maintext->text = g_strdup_printf("%s", executable->display_name);

		/* select a background */
		gchar* filepath = meh_db_get_executable_resource_path(app->db, executable, "screenshot");
		if (filepath == NULL) { /* try fanarts if no screenshots available */
			meh_db_get_executable_resource_path(app->db, executable, "fanart");
		}
		if (filepath != NULL) {
			data->background = meh_image_load_file(app->window->sdl_renderer, filepath);
			meh_widget_image_set_texture(data->background_widget, data->background, FALSE);
		}

		gchar* dt = meh_displayable_datetime(executable->last_played);
		data->subtext->text = g_strdup_printf("Last started on %s", dt);
		g_free(dt);
	}

	meh_widget_text_reload(app->window, data->maintext);
	meh_widget_text_reload(app->window, data->subtext);
	data->maintext->x = meh_transition_start(MEH_TRANSITION_CUBIC, MEH_FAKE_WIDTH+200, 320, 300);
	meh_screen_add_text_transitions(screen, data->maintext);
	data->subtext->x = meh_transition_start(MEH_TRANSITION_CUBIC, MEH_FAKE_WIDTH+200, 325, 550);
	meh_screen_add_text_transitions(screen, data->subtext);
}

int meh_screen_platform_list_update(Screen* screen) {
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	g_assert(data != NULL);

	/* updates all the transition in the screen */
	meh_screen_update_transitions(screen);

	return 0;
}

int meh_screen_platform_list_render(App* app, Screen* screen, gboolean flip) {
	g_assert(app != NULL);
	g_assert(screen != NULL);

	PlatformListData* data = meh_screen_platform_list_get_data(screen);
	g_assert(data != NULL);

	SDL_Color black = { 0, 0, 0 };
	int platform_count = g_queue_get_length(data->platforms);

	/* clear the screen */
	meh_window_clear(app->window, black);
	
	/* background image */
	if (data->background != NULL) {
		meh_widget_image_render(app->window, data->background_widget);
	}

	/* background hover */
	meh_widget_rect_render(app->window, data->background_hover);
	/* selection hover */
	meh_widget_rect_render(app->window, data->hover);

	meh_widget_text_render(app->window, data->maintext);
	meh_widget_text_render(app->window, data->subtext);
	
	if (platform_count == 0) {
		meh_widget_text_render(app->window, data->no_platforms_widget);
	}

	/* icon */
	for (unsigned int i = 0; i < g_queue_get_length(data->icons_widgets); i++) {
		WidgetImage* widget = g_queue_peek_nth(data->icons_widgets, i);
		meh_widget_image_render(app->window, widget);
	}

	/* extra icons */
	if (data->last_started.widget_icon != NULL) {
		meh_widget_image_render(app->window, data->last_started.widget_icon);
	}

	meh_widget_text_render(app->window, data->title);
	
	if (flip == TRUE) {
		meh_window_render(app->window);
	}

	return 0;
}
