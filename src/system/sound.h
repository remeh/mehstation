/*
 * mehstation - Sound.
 *
 * Copyright © 2016 Rémy Mathieu
 */

#pragma once

#include <glib-2.0/glib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct {
	gchar* filename;

	/* ffmpeg part */

	AVFormatContext* fc;
	/* id of the stream in the format context */
	int stream_id;

	/* codec context inside the stream */
	AVCodecContext* stream_codec_ctx;
	/* codec context we've created to use */
	AVCodecContext* codec_ctx;

	AVCodec* codec;

	AVFrame* frame;
} Sound;

Sound* meh_sound_new();
void meh_sound_destroy(Sound* sound);
