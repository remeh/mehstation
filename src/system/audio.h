/*
 * mehstation - Audio engine.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#pragma once

#include <SDL2/SDL.h>
#include <glib-2.0/glib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "system/sound.h"

#define SFX_BIP 0
#define SFX_END 1

typedef struct {
	/* sounds to play, entries in this queue is
	 * owned by the audio engine. */
	GQueue* sounds;

	Sound** soundbank; /* sounds bank */

	SDL_mutex* mutex;

	SDL_AudioDeviceID device_id;
	SDL_AudioSpec spec;

	SDL_Thread* thread;
	gboolean thread_running;
} Audio;

Audio* meh_audio_new();
void meh_audio_destroy(Audio* audio);

void meh_audio_play(Audio* audio, guint sound);
void meh_audio_play_sound(Audio* audio, Sound* sound);
int meh_audio_start(void* audio);
