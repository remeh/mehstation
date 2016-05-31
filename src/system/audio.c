/*
 * mehstation - Audio engine.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include "system/audio.h"
#include "system/sound.h"

static int meh_audio_source_play(void* param);

Audio* meh_audio_new() {
	Audio* audio = g_new(Audio, 1);	

	audio->sounds = g_queue_new();

	for (int i = 0; i < MEH_AUDIO_SOURCES_COUNT; i++) {
		audio->sources[i] = NULL;
	}

	/* open the audio device */

	g_debug("audio: initialization.");
	audio->device = alcOpenDevice(NULL);
	if (audio->device == NULL) {
		g_critical("can't open any audio device.");
		return audio;
	}

	/* create the openal context */

	audio->context = alcCreateContext(audio->device, NULL);
	alcMakeContextCurrent(audio->context);
	alcProcessContext(audio->context);

	if (alGetError() != AL_NO_ERROR) {
		g_critical("error while creating the audio context.");
		return audio;
	}

	g_debug("audio: device+context init ok.");

	/* create the sources */

	for (int i = 0; i < MEH_AUDIO_SOURCES_COUNT; i++) {
		audio->sources[i] = meh_audio_source_new();
	}

	/* listener position to default 0,0,0 */
	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);

	if (alGetError() != AL_NO_ERROR) {
		g_critical("audio: unable to set the listener position.");
	}

	/* loads the sound bank */

	audio->soundbank = g_new(Sound*, SFX_END);
	audio->soundbank[0] = meh_sound_new("res/bip.ogg", TRUE);

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

	/* close the OpenAL device and context */
	if (audio->context != NULL) {
		alcDestroyContext(audio->context);
		audio->context = NULL;
	}
	if (audio->device != NULL) {
		alcCloseDevice(audio->device);
		audio->device = NULL;
	}

	// TODO(remy): test the GDestroyNotify cast
	g_queue_free_full(audio->sounds, (GDestroyNotify)meh_sound_destroy);

	/* delete the sources */
	for (int i = 0; i < MEH_AUDIO_SOURCES_COUNT; i++) {
		meh_audio_source_destroy(audio->sources[i]);
	}

	g_free(audio);
}

void meh_audio_play_sound(Audio* audio, Sound* sound) {
	g_assert(audio != NULL);
	g_assert(sound != NULL);

	for (int i = 0; i < MEH_AUDIO_SOURCES_COUNT; i++) {
		if (!audio->sources[i]->playing) {
			meh_audio_source_play_sound(audio->sources[i], sound);
			break;
		}
	}
}

void meh_audio_play(Audio* audio, guint sound) {
	if (audio == NULL) {
		return;
	}

	if (sound >= SFX_END) {
		return;
	}

	Sound* s = audio->soundbank[sound];

	if (s == NULL) {
		return;
	}

	meh_audio_play_sound(audio, s);
}

AudioSource* meh_audio_source_new() {
	AudioSource* source = g_new(AudioSource, 1);

	source->id = 0;
	source->buffer_id = 0;
	source->playing = FALSE;

	alGenSources(1, &source->id); 
	if (alGetError() != AL_NO_ERROR) {
		g_critical("audio: unable to generate an audio source.");
	}

	alGenBuffers(1, &source->buffer_id);
	if (alGetError() != AL_NO_ERROR) {
		g_critical("audio: unable to generate an audio buffer.");
	}

	g_debug("new source %d, bufid: %d", source->id, source->buffer_id);

	alSource3f(source->id, AL_POSITION, 0.0f, 0.0f, 0.0f);
	return source;
}

void meh_audio_source_destroy(AudioSource* source) {
	g_assert(source != NULL);
	
	if (source->buffer_id > 0) {
		alDeleteBuffers(1, &source->buffer_id);
	}

	if (source->id > 0) {
		alDeleteSources(1, &source->id);
	}

	g_free(source);
}

static int meh_audio_source_play(void* param) {
	AudioSource* source = (AudioSource*)param;

	int state = AL_PLAYING;
	while (state == AL_PLAYING) {
		alGetSourcei(source->id, AL_SOURCE_STATE, &state);
		SDL_Delay(50); // NOTE(remy): arbitrary value
	}

	source->playing = FALSE;

	return 0;
}

void meh_audio_source_play_sound(AudioSource* source, Sound* sound) {
	g_assert(source != NULL);
	g_assert(sound != NULL);
	g_assert(source->playing != TRUE);

	g_debug("audio: source %d playing sound %s", source->id, sound->filename);

	source->playing = TRUE;

	int format = AL_FORMAT_MONO16;
	if (sound->channels > 1) {
		format = AL_FORMAT_STEREO16;
	}

	/* we need to rebind the buffer to the new sound */
	if (source->sound != sound) {
		/* we need to unbind the last used buffer on this source. */
		alSourcei(source->id, AL_BUFFER, 0);

		/* put the sound data into the buffer */
		alBufferData(source->buffer_id, format, sound->data, sound->data->len, sound->sample_rate);

		/* remember which sound this source is playing */
		source->sound = sound;

		int error = alGetError();
		if (error != AL_NO_ERROR) {
			g_critical("error on alBufferData() call: %d", error);
		}

		// attach the buffer to the source
		alSourcei(source->id, AL_BUFFER, source->buffer_id);
	}

	// play
	alSourcePlay(source->id); 

	// launch the thread actually reading the sound
	SDL_CreateThread(&meh_audio_source_play, "source_play", source);
}
