#include <glib-2.0/glib.h>
#include <sqlite3.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "system/db.h"
#include "system/db/models.h"

/*
 * meh_db_open_or_create uses the given filename to open
 * or create (if needed) an SQLite3 DB.
 */
DB* meh_db_open_or_create(const char* filename) {
	DB* db = g_new(DB, 1);

	db->filename = filename;
	/* opens/creates the given filename. */
	int return_code = sqlite3_open_v2(db->filename, &(db->sqlite), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL); 

	if (return_code != SQLITE_OK) {
		g_critical("Can't open the SQLite database with filename '%s', error: %s\n", filename, sqlite3_errstr(return_code));
		return NULL;
	}

	return db;
}

void meh_db_close(DB* db) {
	if (db == NULL) {
		return;
	}

	if (db->sqlite != NULL) {
		sqlite3_close_v2(db->sqlite);
	}
}


/*
 * meh_db_get_platforms gets in the SQLite3 database all the available platforms.
 */
GSList* meh_db_get_platforms(DB* db) {
	g_assert(db != NULL);

	GSList* list = NULL;
	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"name\", \"command\" FROM system";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	if (statement == NULL) {
		g_critical("Null statement returned on : %s\n", sql);
		return NULL;
	}

	/*
	 * read every row
	 */
	while (sqlite3_step(statement) == SQLITE_ROW) {
		/* read column */
		int id = sqlite3_column_int(statement, 0);
		const char* name = (const char*)sqlite3_column_text(statement, 1);	
		const char* command = (const char*)sqlite3_column_text(statement, 2);
		/* build the object */
		Platform* platform = meh_model_platform_new(id, name, command);
		/* append in the list */
		list = g_slist_append(list, platform);
	}

	sqlite3_finalize(statement);

	return list;
}

/*
 * meh_db_get_platform gets in the SQLite3 database one platform.
 */
Platform* meh_db_get_platform(DB* db, int platform_id) {
	g_assert(db != NULL);
	g_assert(platform_id > -1);

	Platform* platform = NULL;
	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"name\", \"command\" FROM system WHERE id = ?1";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_int(statement, 1, platform_id);

	if (statement == NULL) {
		g_critical("Null statement returned on : %s\n", sql);
		return NULL;
	}

	/*
	 * read every row
	 */
	if (sqlite3_step(statement) == SQLITE_ROW) {
		/* read column */
		int id = sqlite3_column_int(statement, 0);
		const char* name = (const char*)sqlite3_column_text(statement, 1);
		const char* command = (const char*)sqlite3_column_text(statement, 2);
		/* build the object */
		platform = meh_model_platform_new(id, name, command);
	}

	sqlite3_finalize(statement);

	return platform;
}

/*
 * meh_db_get_platform_executables gets in  the SQLite3 database all the executables
 * available for the given platform.
 */
GSList* meh_db_get_platform_executables(DB* db, const Platform* platform) {
	g_assert(db != NULL);
	g_assert(platform != NULL);

	GSList* executables = NULL;
	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"display_name\", \"filepath\" FROM executable WHERE platform_id = ?1";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_int(statement, 1, platform->id);

	/*
	 * read every row
	 */
	while (sqlite3_step(statement) == SQLITE_ROW) {
		/* read column */
		int id = sqlite3_column_int(statement, 0);
		const char* display_name = (const char*)sqlite3_column_text(statement, 1);	
		const char* filepath = (const char*)sqlite3_column_text(statement, 2);
		/* build the object */
		Executable* executable = meh_model_executable_new(id, display_name, filepath);
		/* append in the list */
		executables = g_slist_append(executables, executable);
	}

	/* finalize our work with this statement. */
	sqlite3_finalize(statement);

	return executables;
}
