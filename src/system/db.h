#pragma once

#include <glib.h>
#include <sqlite3.h>

struct Platform;
struct Executable;
struct Mapping;

typedef struct DB {
	/* filename of the DB to use. */
	gchar* filename;
	/* an opened db. */
	sqlite3* sqlite; 
} DB;

struct App;

DB* meh_db_open_or_create(struct App*);
void meh_db_close(DB* db);
GQueue* meh_db_get_platforms(DB* db);
struct Platform* meh_db_get_platform(DB* db, int platform_id);
struct Executable* meh_db_read_executable(sqlite3_stmt* statement);
struct Executable* meh_db_get_platform_random_executable(DB* db, int platform_id);
struct Executable* meh_db_get_random_executable(DB* db, int* platform_id);
struct Executable* meh_db_get_last_started_executable(DB* db, int *platform_id);;
GQueue* meh_db_get_platform_executables(DB* db, const struct Platform* platform, gboolean get_resources);
int meh_db_count_platform_executables(DB* db, const struct Platform* platform);
struct Executable* meh_db_get_executable(DB* db, int executable_id);
gboolean meh_db_update_executable_last_played(DB* db, const struct Executable* executable);
GQueue* meh_db_get_executable_resources(DB* db, const struct Executable* executable);
gchar* meh_db_get_executable_resource_path(DB* db, const struct Executable* executable, const char* type);
gboolean meh_db_set_executable_favorite(DB* db, const struct Executable* executable, gboolean favorite);
void meh_db_delete_mapping(DB* db, gchar* id);
struct Mapping* meh_db_get_mapping(DB* db, const gchar* id);
void meh_db_save_mapping(DB* db, struct Mapping* mapping);
int meh_db_count_mapping(DB* db);
