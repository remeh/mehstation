#include <string.h>
#include "glib-2.0/glib.h"
#include "SDL2/SDL.h"

#include "settings.h"

/*
 * meh_read_settings opens the given file and read its content
 * to fill the provided settings.
 */
gboolean meh_settings_read(Settings *settings, const char *filename) {
	g_assert(settings != NULL);
	g_assert(filename != NULL);
	g_assert(strlen(filename) > 0);

	/* Test for the existence of the file. */
	gboolean exists = g_file_test(filename, G_FILE_TEST_EXISTS);
	if (!exists) {
		g_critical("The configuration file doesn't exist.");
		return FALSE;
	}

	GKeyFile* keyfile = g_key_file_new();
	GError* error = NULL;
	g_key_file_load_from_file(
			keyfile,
			filename,
			G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
			&error);

	if (error != NULL) {
		g_critical("Error while reading the configuration file: %s\n", error->message);
		g_error_free(error);
	}

	settings->width = meh_settings_read_int(keyfile, "mehstation", "width", 640);
	settings->height = meh_settings_read_int(keyfile, "mehstation", "height", 480);
	settings->fullscreen = meh_settings_read_bool(keyfile, "mehstation", "fullscreen", FALSE);

	settings->input_repeat_delay = meh_settings_read_int(keyfile, "mehstation", "input_repeat_delay", 300);
	settings->input_repeat_frequency = meh_settings_read_int(keyfile, "mehstation", "input_repeat_frequency", 50);

	printf("Input repeat delay: %d\n",settings->input_repeat_delay);
	printf("Input repeat frequency: %d\n",settings->input_repeat_frequency);

	return TRUE;
}

int meh_settings_read_int(GKeyFile* keyfile, const gchar* group_name, const gchar* key, int default_value) {
	g_assert(keyfile != NULL);

	GError* error = NULL;
	int value = g_key_file_get_integer(keyfile, group_name, key, &error);
	if (error != NULL) {
		return default_value;
	}
	return value;
}

gboolean meh_settings_read_bool(GKeyFile* keyfile, const gchar* group_name, const gchar* key, gboolean default_value) {
	g_assert(keyfile != NULL);

	GError* error = NULL;
	gboolean value = g_key_file_get_boolean(keyfile, group_name, key, &error);
	if (error != NULL) {
		return default_value;
	}
	return value;
}

/*
 * Display some system information on the standart output.
 */
void meh_settings_print_system_infos() {
	SDL_DisplayMode display_mode;
	int i;

	g_message("Available displays: %d", SDL_GetNumVideoDisplays());
	for (i = 0; i < SDL_GetNumVideoDisplays(); i++) {
		if (SDL_GetCurrentDisplayMode(i, &display_mode) == 0) {
			g_message("Display #%d: display mode is %dx%d@%dhz.", i, display_mode.w, display_mode.h, display_mode.refresh_rate);
		}
	}

	g_message("Video driver: %s", SDL_GetCurrentVideoDriver());
}
