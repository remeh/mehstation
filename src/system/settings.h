#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	gint width;
	gint height;
	gboolean fullscreen;

	guint input_repeat_delay;
	guint input_repeat_frequency;
} Settings;

gboolean meh_settings_read(Settings *settings, const gchar *filename);
int meh_settings_read_int(GKeyFile* keyfile, const gchar* group_name, const gchar* key, int default_value);
gboolean meh_settings_read_bool(GKeyFile* keyfile, const gchar* group_name, const gchar* key, gboolean default_value);
void meh_settings_print_system_infos();
