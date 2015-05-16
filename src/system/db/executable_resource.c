#include <glib.h>
#include "system/app.h"
#include "system/db/executable_resource.h"
#include "view/image.h"

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

void meh_model_exec_res_list_destroy(GQueue* exec_resources) {
	for (unsigned int i = 0; i < g_queue_get_length(exec_resources); i++) {
		ExecutableResource* exec_res = g_queue_peek_nth(exec_resources, i);
		meh_model_exec_res_destroy(exec_res);
	}
	g_queue_free(exec_resources);
}

/*
 * meh_model_exec_res_as_texture loads the given resource as a SDL_Texture*.
 * Returns NULL if it's not a texture.
 * The returned texture must be freed by the caller.
 */
SDL_Texture* meh_model_exec_res_as_texture(App* app, ExecutableResource* exec_res) {
	g_assert(app != NULL);
	g_assert(app->window != NULL);
	g_assert(app->window->sdl_renderer != NULL);
	g_assert(exec_res != NULL);

	/* Checks that it's an image */
	if (g_strcmp0(exec_res->type, MEH_EXEC_RES_COVER) != 0 &&
		g_strcmp0(exec_res->type, MEH_EXEC_RES_FANART) != 0 &&
		g_strcmp0(exec_res->type, MEH_EXEC_RES_SCREENSHOT) != 0) {
		return NULL;
	}

	return meh_image_load_file(app->window->sdl_renderer, exec_res->filepath);
}
