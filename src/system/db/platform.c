#include <glib.h>

#include "system/db/platform.h"

Platform* meh_model_platform_new(int id, const char* name, const char* command, const char* icon, const char* background) {
	Platform* platform = g_new(Platform, 1);

	platform->id = id;
	platform->name = g_strdup(name);
	platform->command = g_strdup(command);
	platform->icon = g_strdup(icon);
	platform->background = g_strdup(background);

	return platform;
}

void meh_model_platform_destroy(Platform* platform) {
	g_assert(platform != NULL);

	g_free(platform->name);
	g_free(platform->command);
	g_free(platform->icon);
	g_free(platform->background);

	g_free(platform);
}

void meh_model_platforms_destroy(GQueue* platforms) {
	int i = 0;
	for (i = 0; i < g_queue_get_length(platforms); i++) {
		Platform* platform = g_queue_peek_nth(platforms, i);
		meh_model_platform_destroy(platform);
	}
	g_queue_free(platforms);
}
