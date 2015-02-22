#pragma once

#include <glib.h>

typedef struct Executable {
	int id;
	gchar* display_name;
	gchar* filepath;
	GQueue* resources;
} Executable;

Executable* meh_model_executable_new(int id, const gchar* display_name, const gchar* filepath);
void meh_model_executable_destroy(Executable* executable);
void meh_model_executables_destroy(GQueue* executables);
