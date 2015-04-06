#pragma once

#include <glib.h>

typedef struct Mapping {
	gchar* id;

	int up;
	int down;
	int left;
	int right;
	int start;
	int select;
	int a;
	int b;
	int l;
	int r;
} Mapping;

Mapping* meh_model_mapping_new(const gchar* id, int up, int down, int left, int right,
		                       int start, int select, int a, int b, int l, int r);
void meh_model_mapping_destroy(Mapping* mapping);
GHashTable* meh_model_mapping_generate(Mapping* mapping);
