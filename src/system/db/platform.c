#include <glib.h>

#include "system/db/platform.h"

Platform* meh_model_platform_new(int id, const char* name, const char* command, const char* icon, const char* background, const char* type, const char* discover_dir, const char* discover_ext) {
	Platform* platform = g_new(Platform, 1);

	platform->id = id;
	platform->name = g_strdup(name);
	platform->command = g_strdup(command);
	platform->icon = g_strdup(icon);
	platform->background = g_strdup(background);
	platform->type = g_strdup(type);
	platform->discover_dir = g_strdup(discover_dir);
	platform->discover_ext = g_strdup(discover_ext);

	return platform;
}

Platform* meh_model_platform_copy(const Platform* platform) {
	g_assert(platform != NULL);

	return meh_model_platform_new(
		platform->id,
		platform->name,
		platform->command,
		platform->icon,
		platform->background,
		platform->type,
		platform->discover_dir,
		platform->discover_ext
	);
}

void meh_model_platform_destroy(Platform* platform) {
	g_assert(platform != NULL);

	g_free(platform->name);
	g_free(platform->command);
	g_free(platform->icon);
	g_free(platform->background);
	g_free(platform->type);
	g_free(platform->discover_dir);
	g_free(platform->discover_ext);

	g_free(platform);
	platform = NULL;
}

void meh_model_platforms_destroy(GQueue* platforms) {
	for (unsigned int i = 0; i < g_queue_get_length(platforms); i++) {
		Platform* platform = g_queue_peek_nth(platforms, i);
		meh_model_platform_destroy(platform);
	}
	g_queue_free(platforms);
}
