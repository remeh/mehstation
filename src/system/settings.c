#include <string.h>
#include "glib-2.0/glib.h"

#include "settings.h"

gboolean meh_read_settings(Settings *settings, const char *filename) {
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

	settings->width = g_key_file_get_integer(keyfile, "mehstation", "width", NULL);
	settings->height = g_key_file_get_integer(keyfile, "mehstation", "height", NULL);
	settings->fullscreen = g_key_file_get_boolean(keyfile, "mehstation", "fullscreen", NULL);
}
