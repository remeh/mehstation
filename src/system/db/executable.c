#include <string.h>
#include <glib-2.0/glib.h>

#include "system/db/executable.h"
#include "system/db/executable_resource.h"

static gchar* meh_string_copy(const gchar* str, const gchar* fallback);

Executable* meh_model_executable_new(int id, const gchar* display_name, const gchar* filepath,
		const gchar* description, const gchar* genres, const gchar* publisher,
		const gchar* developer, const gchar* release_date, const gchar* rating, const gchar* players,
		const gchar* extra_parameter, gboolean favorite, GDateTime* last_played) {
	Executable* executable = g_new(Executable, 1);

	executable->id = id;
	executable->display_name = g_strdup(display_name);
	executable->filepath = g_strdup(filepath);

	executable->description = meh_string_copy(description, "No description.");
	executable->genres = meh_string_copy(genres, "Unknown");
	executable->publisher = meh_string_copy(publisher, "Unknown");
	executable->developer = meh_string_copy(developer, "Unknown");
	executable->release_date = meh_string_copy(release_date, "Unknown");

	executable->extra_parameter = meh_string_copy(extra_parameter, "");
	executable->favorite = favorite;
	executable->last_played = last_played;

	if (g_strcmp0(rating, "0.0") == 0) {
		executable->rating = g_strdup("No rating");
	} else {
		executable->rating = meh_string_copy(rating, "No rating");
	}

	if (g_strcmp0(players, "0")  == 0) {
		executable->players = g_strdup("Unknown");
	} else {
		executable->players = meh_string_copy(players, "Unknown");
	}

	executable->resources = g_queue_new();

	return executable;
}

Executable* meh_model_executable_copy(Executable* executable) {
	g_assert(executable != NULL);

	return meh_model_executable_new(
		executable->id,
		executable->display_name,
		executable->filepath,
		executable->description,
		executable->genres,
		executable->publisher,
		executable->developer,
		executable->release_date,
		executable->rating,
		executable->players,
		executable->extra_parameter,
		executable->favorite,
		executable->last_played
	);
}

/*
 * meh_string_copy copies the string or fallback to the given value.
 */
static gchar* meh_string_copy(const gchar* str, const gchar* fallback) {
	if (str == NULL || strlen(str) == 0) {
		return g_strdup(fallback);
	}
	return g_strdup(str);
}

void meh_model_executable_destroy(Executable* executable) {
	g_assert(executable != NULL);

	/* destroy the executable resources if any. */
	if (executable->resources != NULL) {
		meh_model_exec_res_list_destroy(executable->resources);
	}

	g_free(executable->display_name);
	g_free(executable->filepath);
	
	g_free(executable->description);
	g_free(executable->genres);
	g_free(executable->publisher);
	g_free(executable->developer);
	g_free(executable->release_date);
	g_free(executable->rating);
	g_free(executable->extra_parameter);

	g_date_time_unref(executable->last_played);

	g_free(executable);
}

void meh_model_executables_destroy(GQueue* executables) {
	for (unsigned int i = 0; i < g_queue_get_length(executables); i++) {
		Executable* executable = g_queue_peek_nth(executables, i);
		meh_model_executable_destroy(executable);
	}
	g_queue_free(executables);
}
