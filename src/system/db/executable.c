#include <glib-2.0/glib.h>

#include "system/db/executable.h"
#include "system/db/executable_resource.h"

Executable* meh_model_executable_new(int id, const gchar* display_name, const gchar* filepath) {
	Executable* executable = g_new(Executable, 1);

	executable->id = id;
	executable->display_name = g_strdup(display_name);
	executable->filepath = g_strdup(filepath);
	executable->resources = NULL;

	return executable;
}

void meh_model_executable_destroy(Executable* executable) {
	g_assert(executable != NULL);

	/* destroy the executable resources if any. */
	if (executable->resources != NULL) {
		meh_model_exec_res_list_destroy(executable->resources);
	}

	g_free(executable->display_name);
	g_free(executable->filepath);
	g_free(executable);
}

void meh_model_executables_destroy(GSList* executables) {
	int i = 0;
	for (i = 0; i < g_slist_length(executables); i++) {
		Executable* executable = g_slist_nth_data(executables, i);
		meh_model_executable_destroy(executable);
	}
	g_slist_free(executables);
}
