#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	gint width;
	gint height;
	gboolean fullscreen;
} Settings;

gboolean meh_settings_read(Settings *settings, const gchar *filename);
