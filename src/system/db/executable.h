#pragma once

#include <glib.h>

typedef struct Executable {
	int id;
	gchar* display_name;
	gchar* filepath;

	gchar* description;
	gchar* genres;
	gchar* players;
	gchar* publisher;
	gchar* developer;
	gchar* release_date;
	gchar* rating;
	gchar* extra_parameter;

	gboolean favorite;
	GDateTime* last_played;

	GQueue* resources;
} Executable;

Executable* meh_model_executable_new(int id, const gchar* display_name, const gchar* filepath,
		const gchar* description, const gchar* genres, const gchar* publisher,
		const gchar* developer, const gchar* release_date, const gchar* rating,
		const gchar* players, const gchar* extra_parameter,
		gboolean favorite, GDateTime* last_played);
Executable* meh_model_executable_copy(Executable *executable);
void meh_model_executable_destroy(Executable* executable);
void meh_model_executables_destroy(GQueue* executables);
