/*
 * mehstation - Audio.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#pragma once

typedef struct {
} Sound;

Sound* meh_sound_new();
void meh_sound_destroy(Sound* sound);
