#include <libavformat/avformat.h>
#include "view/video.h"

static int meh_video_ffmpeg_open(Video* video);

Video* meh_video_new(gchar* filename) {
	Video* video = g_new(Video, 1);

	video->fc = NULL;
	video->codec_ctx = NULL;
	video->stream_codec_ctx = NULL;
	video->codec = NULL;
	video->stream_id = -1;

	/* ensure the data by copying the filename */
	video->filename = g_strdup(filename);

	/* open the video */
	if (meh_video_ffmpeg_open(video) != 0) {
		meh_video_destroy(video);
		return NULL;
	}

	return video;
}

int meh_video_ffmpeg_open(Video* video) {
	/* open the video */

	if (avformat_open_input(&(video->fc), video->filename, NULL, NULL) != 0) {
		g_critical("Error while opening the video '%s'", video->filename);
		return 1;
	}

	if(avformat_find_stream_info(video->fc, NULL) < 0) {
		g_critical("Can't find any stream info for the video '%s'", video->filename);
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
		return 3;
	}

	/* pointer to the codec context in the stream */

	video->stream_codec_ctx = video->fc->streams[video->stream_id]->codec;

	/* find the decoder */

	video->codec = avcodec_find_decoder(video->stream_codec_ctx->codec_id);
	if (video->codec == NULL) {
		g_critical("Unsupported codec for the file '%s'", video->filename);
		return 4;
	}

	/* creates our own context for the reading */

	video->codec_ctx = avcodec_alloc_context3(video->codec);
	if (avcodec_copy_context(video->codec_ctx, video->stream_codec_ctx)) {
		g_critical("Can't create our codec reading context for file '%s'", video->filename);
		return 5;
	}

	/* open the codec */

	if (avcodec_open2(video->codec_ctx, video->codec, NULL) < 0) {
		g_critical("Could not copy the context for the file '%s'", video->filename);
		return 6;
	}

	return 0;
}

void meh_video_destroy(Video* video) {
	g_assert(video != NULL);

	if (video->fc != NULL) {
		avformat_close_input(&(video->fc));
	}
	if (video->codec_ctx != NULL) {
		avcodec_close(video->codec_ctx);
	}
	if (video->stream_codec_ctx != NULL) {
		avcodec_close(video->codec_ctx);
	}

	g_free(video->filename);
	g_free(video);
}
