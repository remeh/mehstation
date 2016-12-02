#pragma once

#include <glib.h>

struct App;

/* return a list of generated executables, the executables
 * and the queue memory must be released by the caller. */
void meh_discover_scan_directory(struct App* app, const Platform* platform);
gboolean meh_discover_has_exec(struct App* app, const Platform *platform);
