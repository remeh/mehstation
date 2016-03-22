/*
 * mehstation - Audio.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include <glib.h>
#include "system/audio.h"

Sound* meh_sound_new(gchar* filename) {

	return NULL;
}

void meh_sound_destroy(Sound* sound) {
	g_assert(sound != NULL);

	g_free(sound);
}
