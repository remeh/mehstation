#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	gint width;
	gint height;
	gboolean fullscreen;
} Settings;

gboolean meh_read_settings(Settings *settings, const gchar *filename);
