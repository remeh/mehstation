/*
 * mehstation - Audio engine.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include "system/settings.h"
#include "system/audio.h"
#include "system/sound.h"

static void meh_audio_stop(Audio* audio, Sound* sound);

Audio* meh_audio_new(Settings settings) {
	Audio* audio = g_new(Audio, 1);	

	audio->sounds = g_queue_new();
	audio->mutex = SDL_CreateMutex();

	if (!audio->mutex) {
		g_critical("can't create the audio mutex.");
	}

	/* open the audio device */

	SDL_AudioSpec want;
	want.channels = 2;
	want.samples = 2048;
	want.callback = NULL;

	audio->device_id = SDL_OpenAudioDevice(
		NULL, 0, &want, &(audio->spec), SDL_AUDIO_ALLOW_FORMAT_CHANGE
	);

	SDL_PauseAudioDevice(audio->device_id, 0);

	/* start the playing thread */

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

	g_message("sound: start playing '%s'", sound->filename);
	g_queue_push_tail(audio->sounds, sound);

	SDL_UnlockMutex(audio->mutex);
}

void meh_audio_stop(Audio* audio, Sound* sound) {
	g_assert(audio != NULL);
	g_assert(sound != NULL);

	SDL_LockMutex(audio->mutex);

	g_message("sound: stop playing '%s'", sound->filename);
	gboolean removed = g_queue_remove(audio->sounds, sound);
	if (removed) {
		meh_sound_destroy(sound);
	}

	SDL_UnlockMutex(audio->mutex);
}

/* meh_audio_start starts the background threading
 * reading the sound to play on the sound card */
int meh_audio_start(void* audio) {
	g_assert(audio != NULL);

	Audio* a = (Audio*)audio;

	while (a->thread_running) {

		/* for each sound */
		for (int i = 0; i < g_queue_get_length(a->sounds); i++) {
			Sound* sound = g_queue_peek_nth(a->sounds, i);

			AVPacket packet;
			gboolean frame_finished = FALSE;
			gboolean stop_sound = TRUE;

			while (av_read_frame(sound->fc, &packet) >= 0) {
				stop_sound = FALSE;

				/* ensure we're dealing with the good stream */
				if (packet.stream_index == sound->stream_id) {
					/* decode the audio frame */
					avcodec_decode_audio4(sound->codec_ctx, sound->frame, &frame_finished, &packet);
					if (frame_finished) {
						/* play the frame */
						SDL_QueueAudio(a->device_id, sound->frame->data[0], sound->frame->linesize[0]);

						av_free_packet(&packet);
						break;

					}
				}
				av_free_packet(&packet);
			}

			if (stop_sound) {
				meh_audio_stop(a, sound);
			}
		}


		SDL_Delay(16);
	}

	g_debug("audio engine is stopping.");
	return 0;
}
