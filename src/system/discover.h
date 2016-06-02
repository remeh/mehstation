#pragma once

#include <glib.h>

/* return a list of generated executables, the executables
 * and the queue memory must be released by the caller. */
GQueue* meh_discover_scan_directory(gchar* filepath, gchar* extensions);
