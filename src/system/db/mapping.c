#include <glib.h>

#include "system/input.h"
#include "system/db/mapping.h"

Mapping* meh_model_mapping_new(const gchar* id, int up, int down, int left, int right,
		                       int start, int select, int a, int b, int l, int r) {
	Mapping* m = g_new(Mapping, 1);

	m->id = g_strdup(id);
    m->up = up;
	m->down = down;
	m->left = left;
	m->right = right;
	m->start = start;
	m->select = select;
	m->a = a;
	m->b = b;
	m->l = l;
	m->r = r;

	m->m = meh_input_create_mapping(up, down, left, right, start, select, a, b, l, r, 0);
	
	return m;
}

void meh_model_mapping_destroy(Mapping* mapping) {
	g_assert(mapping != NULL);

	g_hash_table_destroy(mapping->m);

	g_free(mapping->id);
}

