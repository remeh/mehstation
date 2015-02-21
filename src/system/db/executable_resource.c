#include <glib.h>
#include "system/db/executable_resource.h"

ExecutableResource* meh_model_exec_res_new(int id, int executable_id, const gchar* type, const gchar* filepath) {
	ExecutableResource* exec_res = g_new(ExecutableResource, 1);

	exec_res->id = id;
	exec_res->executable_id = executable_id;
	exec_res->type = g_strdup(type);
	exec_res->filepath = g_strdup(filepath);

	/* TODO Maybe we should load the real image data here ? */

	return exec_res;
}

void meh_model_exec_res_destroy(ExecutableResource* exec_res) {
	g_assert(exec_res != NULL);

	g_free(exec_res->type);
	g_free(exec_res->filepath);

	g_free(exec_res);
}

void meh_model_exec_res_list_destroy(GSList* exec_resources) {
	int i = 0;
	for (i = 0; i < g_slist_length(exec_resources); i++) {
		ExecutableResource* exec_res = g_slist_nth_data(exec_resources, i);
		meh_model_exec_res_destroy(exec_res);
	}
	g_slist_free(exec_resources);
}

