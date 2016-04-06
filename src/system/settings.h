#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	/* mehstation */
	gchar* name;
	gint width;
	gint height;
	gboolean fullscreen;
	/* input */
	guint input_repeat_delay;
	guint input_repeat_frequency;
	/* render */
	guint max_update_per_second;
	guint max_frameskip;
	guint fade_duration;
	gboolean zoom_logo;
	/* sound */
	gboolean sfx;
} Settings;

gboolean meh_settings_read(Settings *settings, const gchar *filename);
gchar* meh_settings_read_string(GKeyFile* keyfile, const gchar* group_name, const gchar* key, gchar* default_value);
int meh_settings_read_int(GKeyFile* keyfile, const gchar* group_name, const gchar* key, int default_value);
gboolean meh_settings_read_bool(GKeyFile* keyfile, const gchar* group_name, const gchar* key, gboolean default_value);
