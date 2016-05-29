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

#ifdef __APPLE__
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include "system/sound.h"

#define SFX_BIP 0
#define SFX_END 1

#define MEH_AUDIO_SOURCES_COUNT 8

/*
 * Audio Source
 */

typedef struct {
	ALuint id;
	ALuint buffer_id;

	gboolean playing;

	/* to remember which sound this source is playing
	 * (for example to deal with loop playing),
	 * Don't release this memory.
	 */
	Sound* sound;
} AudioSource;

AudioSource* meh_audio_source_new();
void meh_audio_source_destroy(AudioSource* source);
void meh_audio_source_play_sound(AudioSource* source, Sound* sound);

/*
 * Audio System
 */

typedef struct {
	/* sounds to play, entries in this queue is
	 * owned by the audio engine. */
	GQueue* sounds;

	Sound** soundbank; /* sounds bank */

	// ----------------------

	AudioSource* sources[MEH_AUDIO_SOURCES_COUNT];
	ALCdevice* device;
	ALCcontext* context;
} Audio;

Audio* meh_audio_new();
void meh_audio_destroy(Audio* audio);

void meh_audio_play(Audio* audio, guint sound);
void meh_audio_play_sound(Audio* audio, Sound* sound);
int meh_audio_start(void* audio);

