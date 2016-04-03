/*
 * mehstation - Audio engine.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include "system/audio.h"
#include "system/sound.h"

static void meh_audio_stop(Audio* audio, Sound* sound);

Audio* meh_audio_new() {
	Audio* audio = g_new(Audio, 1);	

	audio->sounds = g_queue_new();
	audio->mutex = SDL_CreateMutex();

	if (!audio->mutex) {
		g_critical("can't create the audio mutex.");
	}

	/* open the audio device */

	SDL_AudioSpec want;
	want.channels = 2;
	want.samples = 128;
	want.callback = NULL;

	audio->device_id = SDL_OpenAudioDevice(
		NULL, 0, &want, &(audio->spec), SDL_AUDIO_ALLOW_FORMAT_CHANGE
	);

	SDL_PauseAudioDevice(audio->device_id, 0);

	/* start the playing thread */

	audio->thread_running = TRUE;
	audio->thread = SDL_CreateThread(meh_audio_start, "audio", audio);

	/* loads the sound bank */

	audio->soundbank = g_new(Sound*, SFX_END);
	audio->soundbank[0] = meh_sound_new("res/bip.wav", TRUE);

	return audio;
}

void meh_audio_destroy(Audio* audio) {
	g_assert(audio != NULL);

	/* free each sounds */
	if (audio->soundbank != NULL) {
		for (int i = 0; i < SFX_END; i++) {
			if (audio->soundbank[i] != NULL) {
				meh_sound_destroy(audio->soundbank[i]);
			}
		}
		g_free(audio->soundbank);
		audio->soundbank = NULL;
	}

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

void meh_audio_play_sound(Audio* audio, Sound* sound) {
	if (!sound) {
		return;
	}

	SDL_LockMutex(audio->mutex);

	g_debug("sound: start playing '%s'", sound->filename);
	g_queue_push_tail(audio->sounds, sound);

	SDL_UnlockMutex(audio->mutex);
}

void meh_audio_stop(Audio* audio, Sound* sound) {
	g_assert(audio != NULL);
	g_assert(sound != NULL);

	SDL_LockMutex(audio->mutex);

	gboolean removed = g_queue_remove(audio->sounds, sound);
	if (removed) {
		meh_sound_destroy(sound);
	}

	SDL_UnlockMutex(audio->mutex);
}

void meh_audio_play(Audio* audio, guint sound) {
	g_assert(audio != NULL);

	if (sound >= SFX_END) {
		return;
	}

	Sound* s = audio->soundbank[sound];

	if (s == NULL) {
		return;
	}

	meh_audio_play_sound(audio, s);
}

/* meh_audio_start starts the background threading
 * reading the sound to play on the sound card */
int meh_audio_start(void* audio) {
	g_assert(audio != NULL);

	Audio* a = (Audio*)audio;

	while (a->thread_running) {

		/* for each sound */
		for (int i = 0; i < g_queue_get_length(a->sounds); i++) {
			Sound* s = g_queue_peek_nth(a->sounds, i);
			/* NOTE(remy): I think we'll need to copy
			 * the bytes here and probably copy batch
			 * per batch and not the full data at once.*/
			SDL_QueueAudio(a->device_id, s->data->data, s->data->len);
		}

		Sound* sound = NULL;
		while ((sound = g_queue_pop_tail(a->sounds)) != NULL) {
			meh_audio_stop(a, sound);
		}

		SDL_PauseAudioDevice(a->device_id, 0);
		SDL_Delay(1);
	}

	g_debug("audio engine is stopping.");
	return 0;
}
