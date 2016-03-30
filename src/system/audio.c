/*
 * mehstation - Audio engine.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include "system/settings.h"
#include "system/audio.h"
#include "system/sound.h"

Audio* meh_audio_new(Settings settings) {
	Audio* audio = g_new(Audio, 1);	

	audio->sounds = g_queue_new();
	audio->mutex = SDL_CreateMutex();

	if (!audio->mutex) {
		g_critical("can't create the audio mutex.");
	}

	audio->thread_running = TRUE;
	audio->thread = SDL_CreateThread(meh_audio_start, "audio", audio);

	return audio;
}

void meh_audio_destroy(Audio* audio) {
	g_assert(audio != NULL);

	if (audio->thread) {
		audio->thread_running = FALSE;
		SDL_WaitThread(audio->thread, NULL);
		audio->thread = NULL;
	}

	if (audio->mutex) {
		SDL_DestroyMutex(audio->mutex);
		audio->mutex = NULL;
	}

	// TODO(remy): test the GDestroyNotify cast
	g_queue_free_full(audio->sounds, (GDestroyNotify)meh_sound_destroy);
	g_free(audio);
}

void meh_audio_play(Audio* audio, Sound* sound) {
	if (!sound) {
		return;
	}

	SDL_LockMutex(audio->mutex);

	g_queue_push_tail(audio->sounds, sound);

	SDL_UnlockMutex(audio->mutex);
}

/* meh_audio_start starts the background threading
 * reading the sound to play on the sound card */
int meh_audio_start(void* audio) {
	g_assert(audio != NULL);

	Audio* a = (Audio*)audio;

	while (a->thread_running) {
		SDL_Delay(100);
	}

	g_debug("audio engine is stopping.");
	return 0;
}
