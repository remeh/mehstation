#include <glib.h>
#include <sqlite3.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "system/db.h"
#include "system/input.h"
#include "system/db/models.h"

#define MEH_SCHEMA_FILE "res/schema.sql"

static gboolean meh_db_check_schema(DB* db);
static gboolean meh_db_initialize(DB* db);

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

	/* Initialize the database if needed. */
	if (!meh_db_check_schema(db)) {
		g_message("Creating the initial schema in database.");

		gboolean creation_success = meh_db_initialize(db);
		if (creation_success == FALSE) {
			g_critical("Can't initialize the mehstation database.");
			return NULL;
		}
	}

	return db;
}

/*
 * meh_db_close closes the given db.
 */
void meh_db_close(DB* db) {
	g_assert(db != NULL);

	if (db->sqlite != NULL) {
		sqlite3_close_v2(db->sqlite);
	}
}

/*
 * meh_db_initialize initialize the mehstation database.
 */
static gboolean meh_db_initialize(DB* db) {
	g_assert(db != NULL);
	sqlite3_stmt *statement = NULL;

	/* Test for the existence of the file. */
	gboolean exists = g_file_test(MEH_SCHEMA_FILE, G_FILE_TEST_EXISTS);
	if (!exists) {
		g_critical("The schema file doesn't exist.");
		return FALSE;
	}

	gchar* content;
	gchar** queries;
	gsize length;
	GError* error = NULL;

	/* read the queries file */
	g_file_get_contents(MEH_SCHEMA_FILE, &content, &length, &error);

	if (error != NULL) {
		g_critical("Error while reading the schema file : %s\n", error->message);
		g_error_free(error);
		return FALSE;
	}
	
	queries = g_strsplit(content, ";", -1);
	int idx = 0;
	const char* query = NULL;
	while ((query = queries[idx]) != NULL) {
		/* This test because after the last query there is a ; */
		if (strlen(query) != 1) {
			/* Execute the query */
			int return_code = sqlite3_prepare_v2(db->sqlite, query, strlen(query), &statement, NULL);

			if (return_code != SQLITE_OK) {
				g_critical("Can't execute the initialization query: %s", sqlite3_errmsg(db->sqlite));
				sqlite3_finalize(statement);
				return FALSE;
			}

			return_code = sqlite3_step(statement);
			if (return_code != SQLITE_DONE) {
				g_critical("Can't execute the initialization query: %s", sqlite3_errmsg(db->sqlite));
				return FALSE;
			}
		}
		idx++;
		sqlite3_finalize(statement);
		statement = NULL;
	}

	g_strfreev(queries);
	g_free(content);
	sqlite3_finalize(statement);
	g_message("Database initialized.");
	return TRUE;
}

/*
 * meh_db_check_schema checks that the schema is created in the DB.
 */
static gboolean meh_db_check_schema(DB* db) {
	g_assert(db != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"value\" FROM mehstation WHERE \"name\" = \"schema\"";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);

	if (return_code != SQLITE_OK) {
		sqlite3_finalize(statement);
		return FALSE;
	}

	sqlite3_finalize(statement);
	return TRUE;
}

/*
 * meh_db_get_platforms gets in the SQLite3 database all the available platforms.
 */
GQueue* meh_db_get_platforms(DB* db) {
	g_assert(db != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"name\", \"command\", \"icon\", \"background\" FROM platform ORDER BY name";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	if (statement == NULL) {
		g_critical("Null statement returned on : %s\n", sql);
		return NULL;
	}

	GQueue* list = g_queue_new();

	/*
	 * read every row
	 */
	while (sqlite3_step(statement) == SQLITE_ROW) {
		/* read column */
		int id = sqlite3_column_int(statement, 0);
		const char* name = (const char*)sqlite3_column_text(statement, 1);	
		const char* command = (const char*)sqlite3_column_text(statement, 2);
		const char* icon = (const char*)sqlite3_column_text(statement, 3);
		const char* background = (const char*)sqlite3_column_text(statement, 4);
		/* build the object */
		Platform* platform = meh_model_platform_new(id, name, command, icon, background);
		/* append in the list */
		g_queue_push_tail(list, platform);
	}

	sqlite3_finalize(statement);
	return list;
}

/*
 * meh_db_get_mapping looks for the given mapping in the database.
 */
int meh_db_count_mapping(DB* db) {
	g_assert(db != NULL);

	sqlite3_stmt* statement = NULL;
	const char* sql = "SELECT count(\"id\") FROM mapping";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return 0;
	}

	/* can't find this mapping. */
	if (sqlite3_step(statement) != SQLITE_ROW) {
		return 0;
	}

	int count = (int)sqlite3_column_int(statement, 1);

	sqlite3_finalize(statement);
	return count;
}

/*
 * meh_db_get_mapping looks for the given mapping in the database.
 */
GHashTable* meh_db_get_mapping(DB* db, gchar* id) {
	g_assert(db != NULL);
	g_assert(id != NULL);
	g_assert(strlen(id) > 0);

	sqlite3_stmt* statement = NULL;
	const char* sql = "SELECT \"id\", \"left\", \"right\", \"up\", \"down\", \"start\", \"select\", \"a\", \"b\", \"l\", \"r\" FROM mapping WHERE \"id\" = ?1";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_text(statement, 1, id, -1, NULL);

	/* can't find this mapping. */
	if (sqlite3_step(statement) != SQLITE_ROW) {
		return NULL;
	}

	GHashTable* mapping = g_hash_table_new(g_int_hash, g_int_equal);

	int* left = g_new(int, 1); *left = (int)sqlite3_column_int(statement, 1);
	int* button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_LEFT;
	g_hash_table_insert(mapping, left, button_mapping);

	int* right = g_new(int, 1); *right = (int)sqlite3_column_int(statement, 2);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_RIGHT;
	g_hash_table_insert(mapping, right, button_mapping);

	int* up = g_new(int, 1); *up = (int)sqlite3_column_int(statement, 3);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_UP;
	g_hash_table_insert(mapping, up, button_mapping);

	int* down = g_new(int, 1); *down = (int)sqlite3_column_int(statement, 4);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_DOWN;
	g_hash_table_insert(mapping, down, button_mapping);

	int* start = g_new(int, 1); *start = (int)sqlite3_column_int(statement, 5);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_START;
	g_hash_table_insert(mapping, start, button_mapping);

	int* select = g_new(int, 1); *select = (int)sqlite3_column_int(statement, 6);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_SELECT;
	g_hash_table_insert(mapping, select, button_mapping);

	int* a = g_new(int, 1); *a = (int)sqlite3_column_int(statement, 7);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_A;
	g_hash_table_insert(mapping, a, button_mapping);

	int* b = g_new(int, 1); *b = (int)sqlite3_column_int(statement, 8);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_B;
	g_hash_table_insert(mapping, b, button_mapping);

	int* l = g_new(int, 1); *l = (int)sqlite3_column_int(statement, 9);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_L;
	g_hash_table_insert(mapping, l, button_mapping);

	int* r = g_new(int, 1); *r = (int)sqlite3_column_int(statement, 10);
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_R;
	g_hash_table_insert(mapping, r, button_mapping);


	sqlite3_finalize(statement);
	return mapping;
}

/*
 * meh_db_get_platform gets in the SQLite3 database one platform.
 */
Platform* meh_db_get_platform(DB* db, int platform_id) {
	g_assert(db != NULL);
	g_assert(platform_id > -1);

	Platform* platform = NULL;
	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"name\", \"command\", \"icon\", \"background\" FROM platform WHERE id = ?1 ORDER BY name";
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
		const char* icon = (const char*)sqlite3_column_text(statement, 3);
		const char* background = (const char*)sqlite3_column_text(statement, 4);
		/* build the object */
		platform = meh_model_platform_new(id, name, command, icon, background);
	}

	sqlite3_finalize(statement);

	return platform;
}

/*
 * meh_db_get_platform_executables gets in  the SQLite3 database all the executables
 * available for the given platform.
 */
GQueue* meh_db_get_platform_executables(DB* db, const Platform* platform, gboolean get_resources) {
	g_assert(db != NULL);
	g_assert(platform != NULL);

	GQueue* executables = g_queue_new();
	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"display_name\", \"filepath\", \"description\", \"genres\", \"publisher\", \"developer\", \"release_date\", \"rating\", \"players\"  FROM executable WHERE platform_id = ?1 ORDER BY display_name";
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
		const char* description = (const char*)sqlite3_column_text(statement, 3);
		const char* genres = (const char*)sqlite3_column_text(statement, 4);
		const char* publisher = (const char*)sqlite3_column_text(statement, 5);
		const char* developer = (const char*)sqlite3_column_text(statement, 6);
		const char* release_date = (const char*)sqlite3_column_text(statement, 7);
		const char* rating = (const char*)sqlite3_column_text(statement, 8);
		const char* players = (const char*)sqlite3_column_text(statement, 9);

		/* build the object */
		Executable* executable = meh_model_executable_new(id, display_name, filepath, description,
				genres, publisher, developer, release_date, rating, players);

		if (executable != NULL) {
			/* do we get the resources of this executable ? */
			if (get_resources == TRUE) {
				executable->resources = meh_db_get_executable_resources(db, executable);
			}
			/* append in the list */
			g_queue_push_tail(executables, executable);
		}
	}

	/* finalize our work with this statement. */
	sqlite3_finalize(statement);

	return executables;
}

/*
 * meh_db_get_executable_resources gets in  the SQLite3 database all the resources
 * available for the given executable.
 */
GQueue* meh_db_get_executable_resources(DB* db, const Executable* executable) {
	g_assert(db != NULL);
	g_assert(executable != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"executable_id\", \"type\", \"filepath\" FROM executable_resource WHERE executable_id = ?1";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s\n", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	GQueue* exec_resources = g_queue_new();

	sqlite3_bind_int(statement, 1, executable->id);

	/*
	 * read every row
	 */
	while (sqlite3_step(statement) == SQLITE_ROW) {
		/* read column */
		int id = sqlite3_column_int(statement, 0);
		int executable_id = sqlite3_column_int(statement, 1);
		const char* type = (const char*)sqlite3_column_text(statement, 2);	
		const char* filepath = (const char*)sqlite3_column_text(statement, 3);
		/* build the object */
		ExecutableResource* exec_res = meh_model_exec_res_new(id, executable_id, type, filepath);
		/* append in the list */
		if (exec_res != NULL) {
			g_queue_push_tail(exec_resources, exec_res);
		}
	}

	/* finalize our work with this statement. */
	sqlite3_finalize(statement);

	return exec_resources;
}
