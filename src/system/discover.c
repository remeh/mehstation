/*
 * mehstation - Directory discover mode.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include <glib.h>
#include <string.h>

#include "system/discover.h"
#include "system/utils.h"
#include "system/db/models.h"

static gboolean meh_discover_ext_is_valid(gchar* filename, gchar* extension);
static GQueue* meh_discover_read_filenames(gchar* directory);

/* TODO(remy): split the extensions per comma */
GQueue* meh_discover_scan_directory(gchar* directory, gchar* extensions) {
	g_assert(directory != NULL);

	GQueue *executables = g_queue_new();

	if (g_utf8_strlen(directory, 1) <= 0) {
		return executables;
	}

	/* look for all files in the given directory */
	GQueue* filenames = meh_discover_read_filenames(directory);

	for (int i = 0; i < g_queue_get_length(filenames); i++) {
		gchar* filename = g_queue_peek_nth(filenames, i);

		if (meh_discover_ext_is_valid(filename, extensions)) {

			Executable* executable = meh_model_executable_new(
				-1,
				meh_clean_filename(filename),
				g_strdup(filename),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				FALSE,
				0 /* FIXME(remy): 0 is not a valid value. */
			);

			g_queue_push_tail(executables, executable);
		}
	}

	g_queue_free_full(filenames, (GDestroyNotify)g_free);
	return executables;
}

static GQueue* meh_discover_read_filenames(gchar* directory) {
	g_assert(directory != NULL);

	GDir *dir = NULL;
	GError *error = NULL;
	const gchar *filename = NULL;
	GQueue* filenames = g_queue_new();

	/* open the directory */

	dir = g_dir_open(directory, 0, &error);
	if (error != NULL) {
		g_critical("discover: can't look for filenames into '%s': %s", directory, error->message);
		return filenames;
	}

	/* read each filename */

	while ((filename = g_dir_read_name(dir))) {
		g_queue_push_tail(filenames, g_strdup(filename));
	}

	g_dir_close(dir);
	return filenames;
}

/* meh_discover_ext_is_valid checks whether the given filename ends
 * with the given extension */
static gboolean meh_discover_ext_is_valid(gchar* filename, gchar* extension) {
	g_assert(filename != NULL);
	g_assert(extension != NULL);

	glong ext_len = g_utf8_strlen(extension, -1);
	glong file_len = g_utf8_strlen(filename, -1);

	if (file_len < ext_len) {
		return FALSE;
	}

	gchar* end = g_utf8_substring(filename, file_len - 3, file_len);
	gchar* low = g_utf8_strdown(end, -1);

	gboolean rv = FALSE;
	if (!g_utf8_collate(low, extension)) {
		rv = TRUE;
	}

	g_free(low);
	g_free(end);
	return rv;
}
