#pragma once

typedef struct Platform {
	int id;
	gchar* name;
	gchar* command;
	gchar* icon;
} Platform;

Platform* meh_model_platform_new(int id, const char* name, const char* command, const char* icon);
void meh_model_platform_destroy(Platform* platform);
void meh_model_platforms_destroy(GQueue* platforms);
