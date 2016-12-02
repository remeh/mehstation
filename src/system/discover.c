/*
 * mehstation - Directory discover mode.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include <glib.h>
#include <string.h>

#include "system/app.h"
#include "system/discover.h"
#include "system/utils.h"
#include "system/db/models.h"

static gboolean meh_discover_ext_is_valid(const gchar* filename, gchar** extension);
static GQueue* meh_discover_read_filenames(gchar* directory);
static void meh_discover_update_platform_executables(App* app, const Platform* platform, GQueue* executables);

/*
 * meh_discover_scan_directory looks for files having one of the given extensions
 * in the given directory.
 * If some are found, it then check if we should add it to the given platform (it
 * is not the case if it has already been discovered for this platform).
 */
void meh_discover_scan_directory(App* app, const Platform* platform) {
	g_assert(platform != NULL);

	GQueue *executables = g_queue_new();

	if (g_utf8_strlen(platform->discover_dir, 1) <= 0) {
		return;
	}

	/* look for all files in the given directory */
	GQueue* filenames = meh_discover_read_filenames(platform->discover_dir);

	/* split extensions */
	gchar** exts = g_strsplit(platform->discover_ext, ",", -1);

	for (int i = 0; i < g_queue_get_length(filenames); i++) {
		gchar* filename = g_queue_peek_nth(filenames, i);

		if (meh_discover_ext_is_valid(filename, exts)) {

			Executable* executable = meh_model_executable_new(
				-1,
				meh_clean_filename(filename),
				g_strdup_printf("%s/%s", platform->discover_dir, filename),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				FALSE,
				g_date_time_new_now_local()
			);

			g_queue_push_tail(executables, executable);
		}
	}

	g_queue_free_full(filenames, (GDestroyNotify)g_free);
	g_strfreev(exts);

	/* updates the platform executables list in db. */
	meh_discover_update_platform_executables(app, platform, executables);

	/* free everything */
	meh_model_executables_destroy(executables);
}

gboolean meh_discover_has_exec(struct App* app, const Platform *platform) {
	g_assert(app != NULL);
	g_assert(platform != NULL);

	if (g_utf8_strlen(platform->discover_dir, 1) <= 0) {
		return FALSE;
	}

	GDir *dir = NULL;
	GError *error = NULL;
	const gchar *filename = NULL;

	/* split extensions */
	gchar** exts = g_strsplit(platform->discover_ext, ",", -1);

	/* open the directory */

	dir = g_dir_open(platform->discover_dir, 0, &error);
	if (error != NULL) {
		g_critical("discover: can't look for filenames into '%s': %s", platform->discover_dir, error->message);
		return FALSE;
	}

	/* read each filename and look for a valid ext */

	gboolean found = FALSE;
	while ((filename = g_dir_read_name(dir))) {
		if (meh_discover_ext_is_valid(filename, exts)) {
			found = TRUE;
			break;
		}
	}

	g_strfreev(exts);
	g_dir_close(dir);
	return found;
}


/*
 * meh_discover_update_platform_executables checks in database for each executable
 * given, if they're already known for this platform.
 * If they're not, they're added.
 */
static void meh_discover_update_platform_executables(App* app, const Platform* platform, GQueue* executables) {
	g_assert(app != NULL);
	g_assert(platform != NULL);
	g_assert(executables != NULL);

	gboolean exists = FALSE;
	Executable* executable = NULL;

	/* TODO(remy): test for error ? */
	sqlite3_exec(app->db->sqlite, "BEGIN TRANSACTION", NULL, NULL, NULL);

	for (int i = 0; i < g_queue_get_length(executables); i++) {
		executable = g_queue_peek_nth(executables, i);

		/* check for existence */
		exists = meh_db_platform_executable_exists(app->db, platform->id, executable->display_name);

		if (!exists) {
			g_debug("Platform '%s' doesn't have executable '%s', insertion.", platform->name, executable->display_name);
			meh_db_executable_insert(app->db, executable, platform->id);
			/* TODO(remy): abort transaction on error */
		}

		/* next */
		exists = FALSE;
		executable = NULL;
	}

	sqlite3_exec(app->db->sqlite, "COMMIT", NULL, NULL, NULL);
}

/*
 * meh_discover_read_filenames creates a queue of string
 * containing all the filenames inside the given directory.
 * The memory of the returned queue must be freed by the caller.
 */
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
 * with one of the given extensions */
static gboolean meh_discover_ext_is_valid(const gchar* filename, gchar** extensions) {
	g_assert(filename != NULL);
	g_assert(extensions != NULL);

	gchar* extension = NULL;
	gboolean valid = FALSE;
	guint i = 0;

	while ((extension = extensions[i]) != NULL && valid != TRUE) {
		extension = g_strstrip(extension);

		glong ext_len = g_utf8_strlen(extension, -1);
		glong file_len = g_utf8_strlen(filename, -1);

		if (file_len < ext_len) {
			return FALSE;
		}

		gchar* end = g_utf8_substring(filename, file_len - ext_len, file_len);
		gchar* low = g_utf8_strdown(end, -1);

		if (g_utf8_collate(low, extension) == 0) {
			valid = TRUE;
		}

		g_free(low);
		g_free(end);

		i++;
	}

	return valid;
}
