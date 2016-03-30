/*
 * mehstation - Audio.
 *
 * NOTE(remy): some refactor must be done
 * to merge the opening of sound/video
 * with ffmpeg because the code is duplicate.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#include <glib.h>
#include "system/sound.h"

static int meh_sound_ffmpeg_open(Sound* sound);
static void meh_sound_internal_destroy(Sound* sound);

Sound* meh_sound_new(gchar* filename) {
	g_assert(filename != NULL);

	Sound* sound = g_new(Sound, 1);

	sound->fc = NULL;
	sound->codec_ctx = NULL;
	sound->stream_codec_ctx = NULL;
	sound->codec = NULL;
	sound->stream_id = -1;
	sound->frame = NULL;

	/* ensure the data by copying the filename */
	sound->filename = g_strdup(filename);

	/* open the sound */
	if (meh_sound_ffmpeg_open(sound) != 0) {
		meh_sound_destroy(sound);
		return NULL;
	}

	return sound;
}

void meh_sound_destroy(Sound* sound) {
	g_assert(sound != NULL);

	meh_sound_internal_destroy(sound);

	g_free(sound->filename);
	g_free(sound);
}

int meh_sound_ffmpeg_open(Sound* sound) {
	g_assert(sound != NULL);

	/* open the sound */

	if (avformat_open_input(&(sound->fc), sound->filename, NULL, NULL) != 0) {
		g_critical("Error while opening the sound '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 1;
	}

	if(avformat_find_stream_info(sound->fc, NULL) < 0) {
		g_critical("Can't find any stream info for the sound '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 2;
	}

	/* find an audio stream in the file */

	for (int i = 0; i < sound->fc->nb_streams; i++) {
		if (sound->fc->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			sound->stream_id = i;
			break;
		}
	}

	if (sound->stream_id == -1) {
		g_critical("Didn't find any stream in the file '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 3;
	}

	/* pointer to the codec context in the stream */

	sound->stream_codec_ctx = sound->fc->streams[sound->stream_id]->codec;

	/* find the decoder */
	sound->codec = avcodec_find_decoder(sound->stream_codec_ctx->codec_id);
	if (sound->codec == NULL) {
		g_critical("Unsupported codec for the file '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 4;
	}

	/* create our own context for the reading */

	sound->codec_ctx = avcodec_alloc_context3(sound->codec);
	if (avcodec_copy_context(sound->codec_ctx, sound->stream_codec_ctx)) {
		g_critical("Can't create our codec reading context for file '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 5;
	}

	/* open the codec */

	if (avcodec_open2(sound->codec_ctx, sound->codec, NULL) < 0) {
		g_critical("Could not copy the context for the file '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 6;
	}

	return 0;
}

void meh_sound_internal_destroy(Sound* sound) {
	g_assert(sound != NULL);

	if (sound->frame != NULL) {
		av_frame_free(&sound->frame);
		sound->frame = NULL;
	}
	if (sound->stream_codec_ctx != NULL) {
		avcodec_close(sound->stream_codec_ctx);
		sound->stream_codec_ctx = NULL;
	}
	if (sound->codec_ctx != NULL) {
		avcodec_free_context(&sound->codec_ctx);
		sound->codec_ctx = NULL;
	}
	if (sound->fc != NULL) {
		avformat_close_input(&(sound->fc));
		sound->fc = NULL;
	}
}
