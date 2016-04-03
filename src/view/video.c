#include <libavformat/avformat.h>

#include "view/video.h"

static int meh_video_ffmpeg_open(Video* video);
static void meh_video_internal_destroy(Video* video);

Video* meh_video_new(Window* window, gchar* filename) {
	g_assert(window != NULL);
	g_assert(filename != NULL);

	if (strlen(filename) == 0) {
		return NULL;
	}

	Video* video = g_new(Video, 1);

	video->texture = NULL;
	video->fc = NULL;
	video->codec_ctx = NULL;
	video->stream_codec_ctx = NULL;
	video->codec = NULL;
	video->stream_id = -1;
	video->frame = NULL;

	/* ensure the data by copying the filename */
	video->filename = g_strdup(filename);

	/* open the video */
	if (meh_video_ffmpeg_open(video) != 0) {
		meh_video_destroy(video);
		return NULL;
	}

	/* create the texture */
	video->texture = SDL_CreateTexture(
				window->sdl_renderer,
				SDL_PIXELFORMAT_YV12,
				SDL_TEXTUREACCESS_STREAMING,
				video->codec_ctx->width,
				video->codec_ctx->height
			);

	if (video->texture == NULL) {
		g_critical("Can't create the internal texture for the video '%s'", filename);
		meh_video_destroy(video);
		return NULL;
	}

	return video;
}

int meh_video_ffmpeg_open(Video* video) {
	/* open the video */

	if (avformat_open_input(&(video->fc), video->filename, NULL, NULL) != 0) {
		g_critical("Error while opening the video '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 1;
	}

	if(avformat_find_stream_info(video->fc, NULL) < 0) {
		g_critical("Can't find any stream info for the video '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 2;
	}

	/* find a video stream in the file */

	for (int i = 0; i < video->fc->nb_streams; i++) {
		if (video->fc->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			video->stream_id = i;
			break;
		}
	}

	if (video->stream_id == -1) {
		g_critical("Didn't find any stream in the file '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 3;
	}

	/* pointer to the codec context in the stream */

	video->stream_codec_ctx = video->fc->streams[video->stream_id]->codec;

	/* find the decoder */
	video->codec = avcodec_find_decoder(video->stream_codec_ctx->codec_id);
	if (video->codec == NULL) {
		g_critical("Unsupported codec for the file '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 4;
	}

	/* create our own context for the reading */

	video->codec_ctx = avcodec_alloc_context3(video->codec);
	if (avcodec_copy_context(video->codec_ctx, video->stream_codec_ctx)) {
		g_critical("Can't create our codec reading context for file '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 5;
	}

	/* open the codec */

	if (avcodec_open2(video->codec_ctx, video->codec, NULL) < 0) {
		g_critical("Could not copy the context for the file '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 6;
	}

	/* allocate the frame */
	video->frame = av_frame_alloc();
	if (video->frame == NULL) {
		g_critical("Can't allocate a frame for the file '%s'", video->filename);
		meh_video_internal_destroy(video);
		return 7;
	}

	return 0;
}

void meh_video_update(Video* video) {
	g_assert(video != NULL);
	g_assert(video->texture != NULL);

	AVPacket packet;
	int frame_finished = 0;

	while (av_read_frame(video->fc, &packet) >= 0) {
		/* ensure we're dealing with the good stream */
		if (packet.stream_index == video->stream_id) {
			/* decode the video frame */
			avcodec_decode_video2(video->codec_ctx, video->frame, &frame_finished, &packet);
			if (frame_finished) {
				/* apply the read data onto the SDL texture */
				SDL_UpdateYUVTexture(
						video->texture,
						NULL,
						video->frame->data[0],
						video->frame->linesize[0],
						video->frame->data[1],
						video->frame->linesize[1],
						video->frame->data[2],
						video->frame->linesize[2]
					);

				av_packet_unref(&packet);
				break;
			}
		}

		av_packet_unref(&packet);
	}
}

void meh_video_internal_destroy(Video* video) {
	g_assert(video != NULL);

	if (video->frame != NULL) {
		av_frame_free(&video->frame);
		video->frame = NULL;
	}
	if (video->stream_codec_ctx != NULL) {
		avcodec_close(video->stream_codec_ctx);
		video->stream_codec_ctx = NULL;
	}
	if (video->codec_ctx != NULL) {
		avcodec_free_context(&video->codec_ctx);
		video->codec_ctx = NULL;
	}
	if (video->fc != NULL) {
		avformat_close_input(&(video->fc));
		video->fc = NULL;
	}

	if (video->texture != NULL) {
		SDL_DestroyTexture(video->texture);
		video->texture = NULL;
	}
}

void meh_video_destroy(Video* video) {
	g_assert(video != NULL);

	meh_video_internal_destroy(video);

	g_free(video->filename);
	g_free(video);
}
