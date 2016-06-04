#pragma once

typedef struct Platform {
	int id;
	gchar* name;
	gchar* command;
	gchar* icon;
	gchar* background;
	gchar* type;
	gchar* discover_dir;
	gchar* discover_ext;
} Platform;

Platform* meh_model_platform_new(int id, const char* name, const char* command, const char* icon, const char* background, const char* type, const char* discover_dir, const char* discover_ext);
Platform* meh_model_platform_copy(const Platform* platform);
void meh_model_platform_destroy(Platform* platform);
void meh_model_platforms_destroy(GQueue* platforms);
