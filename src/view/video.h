/*
 * mehstation - Video.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include <glib-2.0/glib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "view/window.h"

typedef struct Video {
	gchar* filename;

	/* texture used to render the video. */
	SDL_Texture* texture;

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
} Video;

Video* meh_video_new(Window* window, gchar* filename);
void meh_video_update(Video* video);
void meh_video_destroy(Video* video);
