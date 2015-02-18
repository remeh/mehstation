#pragma once

#include <glib-2.0/glib.h>
#include <sqlite3.h>

struct Platform;

typedef struct DB {
	/* filename of the DB to use. */
	const char* filename;
	/* an opened db. */
	sqlite3* sqlite; 
} DB;

DB* meh_db_open_or_create(const char* filename);
void meh_db_close(DB* db);
GSList* meh_db_get_platforms(DB* db);
struct Platform* meh_db_get_platform(DB* db, int platform_id);
GSList* meh_db_get_platform_executables(DB* db, const struct Platform* platform);
