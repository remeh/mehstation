#pragma once

#include "glib-2.0/glib.h"

typedef struct {
	gint width;
	gint height;
	gboolean fullscreen;
} Settings;

/*
 * meh_read_settings opens the given file and read its content
 * to fill the provided settings.
 */
gboolean meh_read_settings(Settings *settings, const gchar *filename);
