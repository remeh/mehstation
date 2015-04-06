#pragma once

#include <glib.h>
#include <sqlite3.h>

struct Platform;
struct Executable;
struct Mapping;

typedef struct DB {
	/* filename of the DB to use. */
	const char* filename;
	/* an opened db. */
	sqlite3* sqlite; 
} DB;

DB* meh_db_open_or_create(const char* filename);
void meh_db_close(DB* db);
GQueue* meh_db_get_platforms(DB* db);
struct Platform* meh_db_get_platform(DB* db, int platform_id);
GQueue* meh_db_get_platform_executables(DB* db, const struct Platform* platform, gboolean get_resources);
GQueue* meh_db_get_executable_resources(DB* db, const struct Executable* executable);
struct Mapping* meh_db_get_mapping(DB* db, const gchar* id);
void meh_db_save_mapping(DB* db, struct Mapping* mapping);
int meh_db_count_mapping(DB* db);
