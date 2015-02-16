#include <glib-2.0/glib.h>

#include "system/db/platform.h"

Platform* meh_model_platform_new(int id, const char* name, const char* command) {
	Platform* platform = g_new(Platform, 1);
	if (platform == NULL) {
		return NULL;
	}

	platform->id = id;
	platform->name = g_strdup(name);
	platform->command = g_strdup(name);

	return platform;
}

void meh_model_platform_destroy(Platform* platform) {
	g_assert(platform != NULL);

	g_free(platform->name);
	g_free(platform->command);
	g_free(platform);
}

void meh_model_platforms_destroy(GSList* platforms) {
	int i = 0;
	for (i = 0; i < g_slist_length(platforms); i++) {
		Platform* platform = g_slist_nth_data(platforms, i);
		meh_model_platform_destroy(platform);
	}
	g_slist_free(platforms);
}
