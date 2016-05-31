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

Sound* meh_sound_new(gchar* filename, gboolean load) {
	g_assert(filename != NULL);

	Sound* sound = g_new(Sound, 1);

	sound->data = NULL;
	sound->fc = NULL;
	sound->codec_ctx = NULL;
	sound->stream_codec_ctx = NULL;
	sound->codec = NULL;
	sound->stream_id = -1;
	sound->frame = NULL;
	sound->channels = 0;
	sound->sample_rate = 0;

	/* ensure the data by copying the filename */
	sound->filename = g_strdup(filename);

	/* open the sound */
	if (meh_sound_ffmpeg_open(sound) != 0) {
		meh_sound_destroy(sound);
		return NULL;
	}

	/* load te file as bytes to store it in RAM uncompressed */
	if (load == TRUE) {
		meh_sound_load(sound);
	}

	return sound;
}

/* FIXME(remy): this works with WAV and MP3 but bug (noise+correct sound) with OGG?! */
void meh_sound_load(Sound* sound) {
	g_assert(sound != NULL);

	sound->data = g_byte_array_new();

	AVPacket packet;
	gboolean frame_finished = FALSE;

	while (av_read_frame(sound->fc, &packet) >= 0) {
		frame_finished = FALSE;

		sound->sample_rate = sound->codec_ctx->sample_fmt;
		if (sound->frame->sample_rate > 0) {
			sound->sample_rate = sound->frame->sample_rate;
		}
		if (sound->frame->channels > 0) {
			sound->channels = sound->frame->channels;
			sound->channels = sound->codec_ctx->channels;
		}

		/* ensure we're dealing with the good stream */
		if (packet.stream_index == sound->stream_id) {
			/* decode the audio frame */
			avcodec_decode_audio4(sound->codec_ctx, sound->frame, &frame_finished, &packet);
			if (frame_finished) {
				int data_size = av_get_bytes_per_sample(sound->codec_ctx->sample_fmt);

				if (data_size > 0) {
					for (int i = 0; i < sound->frame->nb_samples; i++) {
						for (int ch = 0; ch < sound->codec_ctx->channels; ch++) {
							g_byte_array_append(sound->data, sound->frame->data[ch] + data_size*i, data_size);
						}
					}
				}
			}
		}

		av_packet_unref(&packet);
	}

	g_debug("load the sound %s in %d bytes (channels: %d, sample rate: %d)", sound->filename, sound->data->len, sound->channels, sound->sample_rate);
}

void meh_sound_destroy(Sound* sound) {
	g_assert(sound != NULL);

	if (sound->data != NULL) {
		g_byte_array_free(sound->data, TRUE);
		sound->data = NULL;
	}

	/* ffmpeg part */
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

	/* allocate the frame */
	sound->frame = av_frame_alloc();
	if (sound->frame == NULL) {
		g_critical("Can't allocate a frame for the file '%s'", sound->filename);
		meh_sound_internal_destroy(sound);
		return 7;
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
