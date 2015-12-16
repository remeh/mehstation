#include <glib.h>
#include <sqlite3.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "system/app.h"
#include "system/db.h"
#include "system/input.h"
#include "system/db/models.h"

static gboolean meh_db_check_schema(DB* db);
static gboolean meh_db_initialize(App* app, DB* db);

/*
 * meh_db_open_or_create uses the given filename to open
 * or create (if needed) an SQLite3 DB.
 */
DB* meh_db_open_or_create(App* app) {
	DB* db = g_new(DB, 1);

	gchar* conf_dir = meh_app_init_create_dir_conf(app);
	db->filename = g_strdup_printf("%s/database.db", conf_dir);
	g_free(conf_dir);

	/* opens/creates the given filename. */
	int return_code = sqlite3_open_v2(db->filename, &(db->sqlite), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL); 

	if (return_code != SQLITE_OK) {
		g_critical("Can't open the SQLite database with filename '%s', error: %s", db->filename, sqlite3_errstr(return_code));
		return NULL;
	}

	/* Initialize the database if needed. */
	if (!meh_db_check_schema(db)) {
		g_message("Creating the initial schema in database at: %s", db->filename);

		gboolean creation_success = meh_db_initialize(app, db);
		if (creation_success == FALSE) {
			g_critical("Can't initialize the mehstation database.");
			return NULL;
		}
	} else {
		g_message("Using the database file: %s", db->filename);
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
		db->sqlite = NULL;
	}

	g_free(db->filename);
	db->filename = NULL;
	g_free(db);
}

/*
 * meh_db_initialize initialize the mehstation database.
 */
static gboolean meh_db_initialize(App* app, DB* db) {
	g_assert(db != NULL);
	sqlite3_stmt *statement = NULL;

	/* Test for the existence of the file. */
	gchar* path = g_strdup_printf("%s/schema.sql", app->res_dir);
	gboolean exists = g_file_test(path, G_FILE_TEST_EXISTS);
	if (!exists) {
		g_critical("The schema file doesn't exist.");
		g_free(path);
		path = NULL;
		return FALSE;
	}

	gchar* content;
	gchar** queries;
	gsize length;
	GError* error = NULL;

	/* read the queries file */
	g_file_get_contents(path, &content, &length, &error);

	g_free(path);
	path = NULL;

	if (error != NULL) {
		g_critical("Error while reading the schema file : %s", error->message);
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

	const char* sql = "SELECT \"id\", \"name\", \"command\", \"icon\", \"background\", \"type\" FROM platform ORDER BY name";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	if (statement == NULL) {
		g_critical("Null statement returned on : %s", sql);
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
		const char* type = (const char*)sqlite3_column_text(statement, 5);
		/* build the object */
		Platform* platform = meh_model_platform_new(id, name, command, icon, background, type);
		/* append in the list */
		g_queue_push_tail(list, platform);
	}

	sqlite3_finalize(statement);
	return list;
}

/*
 * meh_db_count_platform_executables returns the number of executables
 * configured for the given platform.
 */
int meh_db_count_platform_executables(DB* db, const Platform* platform) {
	g_assert(db != NULL);
	g_assert(platform != NULL);

	sqlite3_stmt* statement = NULL;
	const char* sql = "SELECT count(\"id\") FROM executable WHERE platform_id = ?1";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return 0;
	}

	sqlite3_bind_int(statement, 1, platform->id);

	/* can't find this mapping. */
	if (sqlite3_step(statement) != SQLITE_ROW) {
		return 0;
	}

	int count = (int)sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);
	return count;
}

/*
 * meh_db_count_mapping looks for the given mapping in the database.
 */
int meh_db_count_mapping(DB* db) {
	g_assert(db != NULL);

	sqlite3_stmt* statement = NULL;
	const char* sql = "SELECT count(\"id\") FROM mapping";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return 0;
	}

	/* can't find this mapping. */
	if (sqlite3_step(statement) != SQLITE_ROW) {
		return 0;
	}

	int count = (int)sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);
	return count;
}

/*
 * meh_db_delete_mapping deletes the mapping with the given ID.
 */
void meh_db_delete_mapping(DB* db, gchar* id) {
	g_assert(db != NULL);

	if (id == NULL || strlen(id) == 0) {
		g_warning("meh_db_delete_mapping called with an null or empty ID.");
		return;
	}

	sqlite3_stmt* statement = NULL;
	const char* sql = "DELETE FROM mapping WHERE id = ?1";

	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query :%s\nError :%s", sql, sqlite3_errstr(return_code));
		return;
	}

	sqlite3_bind_text(statement, 1, id, -1, NULL);

	if (sqlite3_step(statement) != SQLITE_ROW) {
		return;
	}

	g_debug("Deleted the mapping: %s", id);
}

/*
 * meh_db_save_mapping saves the key mapping to the DB.
 */
void meh_db_save_mapping(DB* db, Mapping* mapping) {
	g_assert(db != NULL);
	g_assert(mapping != NULL);

	sqlite3_stmt* statement = NULL;
	const char* sql = "INSERT INTO mapping (id, up, down, left, right, `start`, `select`, a, b, l, r) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)";

	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query :%s\nError :%s", sql, sqlite3_errstr(return_code));
		return;
	}

	sqlite3_bind_text(statement, 1, mapping->id, -1, NULL);

	sqlite3_bind_int(statement, 2, mapping->up);
	sqlite3_bind_int(statement, 3, mapping->down);
	sqlite3_bind_int(statement, 4, mapping->left);
	sqlite3_bind_int(statement, 5, mapping->right);
	sqlite3_bind_int(statement, 6, mapping->start);
	sqlite3_bind_int(statement, 7, mapping->select);
	sqlite3_bind_int(statement, 8, mapping->a);
	sqlite3_bind_int(statement, 9, mapping->b);
	sqlite3_bind_int(statement, 10, mapping->l);
	sqlite3_bind_int(statement, 11, mapping->r);

	if (sqlite3_step(statement) != SQLITE_ROW) {
		return;
	}

	g_debug("Stored the mapping: %s", mapping->id);
}

/*
 * meh_db_get_mapping looks for the given mapping in the database.
 */
Mapping* meh_db_get_mapping(DB* db, const gchar* id) {
	g_assert(db != NULL);
	g_assert(id != NULL);
	g_assert(strlen(id) > 0);

	sqlite3_stmt* statement = NULL;
	const char* sql = "SELECT \"id\", \"up\", \"down\", \"left\", \"right\", \"start\", \"select\", \"a\", \"b\", \"l\", \"r\" FROM mapping WHERE \"id\" = ?1";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_text(statement, 1, id, -1, NULL);

	/* can't find this mapping. */
	if (sqlite3_step(statement) != SQLITE_ROW) {
		return NULL;
	}

	const char* read_id = (const char*)sqlite3_column_text(statement, 0);

	Mapping* m = meh_model_mapping_new(read_id,
					(int)sqlite3_column_int(statement, 1),
					(int)sqlite3_column_int(statement, 2), 
					(int)sqlite3_column_int(statement, 3), 
					(int)sqlite3_column_int(statement, 4), 
					(int)sqlite3_column_int(statement, 5), 
					(int)sqlite3_column_int(statement, 6), 
					(int)sqlite3_column_int(statement, 7), 
					(int)sqlite3_column_int(statement, 8), 
					(int)sqlite3_column_int(statement, 9), 
					(int)sqlite3_column_int(statement, 10)
	);

	sqlite3_finalize(statement);

	return m;
}

/*
 * meh_db_get_platform gets in the SQLite3 database one platform.
 */
Platform* meh_db_get_platform(DB* db, int platform_id) {
	g_assert(db != NULL);
	g_assert(platform_id > -1);

	Platform* platform = NULL;
	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"id\", \"name\", \"command\", \"icon\", \"background\", \"type\" FROM platform WHERE id = ?1 ORDER BY name";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (statement == NULL || return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_int(statement, 1, platform_id);

	if (statement == NULL) {
		g_critical("Null statement returned on : %s", sql);
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
		const char* type = (const char*)sqlite3_column_text(statement, 5);
		/* build the object */
		platform = meh_model_platform_new(id, name, command, icon, background, type);
	}

	sqlite3_finalize(statement);

	return platform;
}

/*
 * meh_db_read_executable reads an executable in the given statement.
 */
Executable* meh_db_read_executable(sqlite3_stmt* statement) {
	g_assert(statement != NULL);

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
	const char* extra_parameter = (const char*)sqlite3_column_text(statement, 10);
	gboolean favorite = sqlite3_column_int(statement, 11) > 0 ? TRUE : FALSE;
	GDateTime* last_played = g_date_time_new_from_unix_local(sqlite3_column_int(statement, 12));

	/* build the object */
	Executable* executable = meh_model_executable_new(id, display_name, filepath, description,
			genres, publisher, developer, release_date, rating, players, extra_parameter,
			favorite, last_played);

	return executable;
}

Executable* meh_db_get_executable(DB* db, int executable_id) {
	g_assert(db != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"e1\".\"id\", \"display_name\", \"filepath\", \"description\", \"genres\", \"publisher\", \"developer\", \"release_date\", \"rating\", \"players\",\"extra_parameter\", \"favorite\", \"last_played\" FROM \"executable\" AS \"e1\" where \"e1\".\"id\" = ?1;";

	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_int(statement, 1, executable_id);

	/* read the value */
	Executable* executable = NULL;
	if (sqlite3_step(statement) == SQLITE_ROW) {
		executable = meh_db_read_executable(statement);
	}

	sqlite3_finalize(statement);

	return executable;
}

/*
 * meh_db_get_platform_random_executable gets one random executable in SQLite3 for the given
 * platform.
 */
Executable* meh_db_get_platform_random_executable(DB* db, int platform_id) {
	g_assert(db != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"e1\".\"id\", \"display_name\", \"filepath\", \"description\", \"genres\", \"publisher\", \"developer\", \"release_date\", \"rating\", \"players\",\"extra_parameter\", \"favorite\", \"last_played\" FROM \"executable\" AS \"e1\" where \"platform_id\" = ?1 ORDER BY RANDOM() LIMIT 1;";

	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_int(statement, 1, platform_id);

	/* read the value */
	Executable* executable = NULL;
	if (sqlite3_step(statement) == SQLITE_ROW) {
		executable = meh_db_read_executable(statement);
	}

	sqlite3_finalize(statement);

	return executable;
}

/*
 * meh_db_get_random_executable gets one random executable in SQLite3, platform_id will be
 * set to the id of the executable's platform.
 */
Executable* meh_db_get_random_executable(DB* db, int* platform_id) {
	g_assert(db != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "SELECT \"e1\".\"id\", \"display_name\", \"filepath\", \"description\", \"genres\", \"publisher\", \"developer\", \"release_date\", \"rating\", \"players\",\"extra_parameter\", \"favorite\", \"last_played\", \"platform_id\" FROM \"executable\" AS \"e1\" JOIN \"platform\" ON \"platform\".\"id\" = \"e1\".\"platform_id\" ORDER BY RANDOM() LIMIT 1;";

	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}


	/* read the value */
	Executable* executable = NULL;
	if (sqlite3_step(statement) == SQLITE_ROW) {
		executable = meh_db_read_executable(statement);
		*platform_id = sqlite3_column_int(statement, 13);
	}

	sqlite3_finalize(statement);

	return executable;
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

	const char* sql = "SELECT \"id\", \"display_name\", \"filepath\", \"description\", \"genres\", \"publisher\", \"developer\", \"release_date\", \"rating\", \"players\",\"extra_parameter\", \"favorite\", \"last_played\"  FROM executable WHERE platform_id = ?1 ORDER BY favorite DESC, upper(\"display_name\")";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);
	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return NULL;
	}

	sqlite3_bind_int(statement, 1, platform->id);

	/*
	 * read every row
	 */
	while (sqlite3_step(statement) == SQLITE_ROW) {
		Executable* executable = meh_db_read_executable(statement);

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

gboolean meh_db_set_executable_favorite(DB* db, const Executable* executable, gboolean favorite) {
	g_assert(db != NULL);
	g_assert(executable != NULL);

	sqlite3_stmt *statement = NULL;

	const char* sql = "UPDATE executable SET favorite = ?1 WHERE id = ?2";
	int return_code = sqlite3_prepare_v2(db->sqlite, sql, strlen(sql), &statement, NULL);

	if (return_code != SQLITE_OK) {
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
		return FALSE;
	}

	sqlite3_bind_int(statement, 1, favorite == TRUE ? 1 : 0);
	sqlite3_bind_int(statement, 2, executable->id);

	if (sqlite3_step(statement) == SQLITE_DONE) {
		return TRUE;
	}

	return FALSE;
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
		g_critical("Can't execute the query: %s\nError: %s", sql, sqlite3_errstr(return_code));
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
