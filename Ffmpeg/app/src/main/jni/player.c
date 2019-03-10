#include <com_pierce_ffmpeg_MPlayer.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"lever",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"lever",FORMAT,##__VA_ARGS__);

#include "libyuv.h"

//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"

#define MAX_STREAM 2
struct Player {
    AVFormatContext *input_format_ctx;
    int video_stream_index;
    int audio_stream_index;
    AVCodecContext *input_codec_ctx[MAX_STREAM];
    pthread_t decode_threads[MAX_STREAM];
    ANativeWindow *nativeWindow;
};

void init_input_format_ctx(struct Player *player, const char *input_cstr) {

    av_register_all();
    AVFormatContext *format_ctx = avformat_alloc_context();
    if (avformat_open_input(&format_ctx, input_cstr, NULL, NULL) != 0) {
        LOGE("%s", "open v err");
        return;
    }

    if (avformat_find_stream_info(&format_ctx, NULL)) {
        LOGE("%s", "v infor err")
        return;
    }
    int i = 0;
    for (i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            player->video_stream_index = i;
        }
        if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            player->audio_stream_index = i;
        }
        player->input_format_ctx = format_ctx;


    }

}

void init_codec_context(struct Player *player, int stream_idx) {
    AVFormatContext *format_ctx = player->input_format_ctx;
    LOGI("%s", "init_codec_context begin");
    AVCodecContext *codecContext = format_ctx->streams[stream_idx]->codec;
    LOGI("init_codec_context end");
    AVCodec *codec = avcodec_find_decoder(codecContext->codec_id);
    if (codec == NULL) {
        LOGI("s", "codec null")
        return;
    }
    if (avcodec_open2(codecContext, codec, NULL) < 0) {

        LOGI("%s", "open err");
        return;
    }
    player->input_codec_ctx[stream_idx] = codecContext;

}

void decode_video(struct Player *player, AVPacket *packet) {
    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    ANativeWindow_Buffer outBuffer;
    AVCodecContext *codecContext = player->input_codec_ctx[player->video_stream_index];
    int got_frame;
    avcodec_decode_video2(codecContext, yuv_frame, &got_frame, packet);
    if (got_frame) {
        ANativeWindow_setBuffersGeometry(player->nativeWindow, codecContext->width,
                                         codecContext->height, WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_lock(player->nativeWindow, &outBuffer, NULL);

        avpicture_fill(rgb_frame, outBuffer.bits, AV_PIX_FMT_RGBA, codecContext->width,
                       codecContext->height);
        //YUV->RGBA_8888
        I420ToARGB(yuv_frame->data[0], yuv_frame->linesize[0],
                   yuv_frame->data[2], yuv_frame->linesize[2],
                   yuv_frame->data[1], yuv_frame->linesize[1],
                   rgb_frame->data[0], rgb_frame->linesize[0],
                   codecContext->width, codecContext->height);
        ANativeWindow_unlockAndPost(player->nativeWindow);
        usleep(1000 * 16);
    }

    av_frame_free(&yuv_frame);
    av_frame_free(&rgb_frame);

}

void *decode_data(void *arg) {
    struct Player *player = (struct Player *) arg;
    AVFormatContext *formatContext = player->input_format_ctx;
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(packet));
    int video_frame_count = 0;
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == player->video_stream_index) {
            decode_video(player, packet);
            LOGI("video_frame_count:%d", video_frame_count++);
        }
        av_free_packet(packet);
    }
}


void decode_video_prepare(JNIEnv *env, struct Player *player, jobject surface) {
    player->nativeWindow = ANativeWindow_fromSurface(env, surface);
}

JNIEXPORT void JNICALL Java_com_pierce_ffmpeg_MPlayer_play
        (JNIEnv *env, jobject jobj, jstring input_jstr, jobject surface) {

    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    struct Player *player = (struct Player *) malloc(sizeof(struct Player));

    init_input_format_ctx(player, input_cstr);
    int video_stream_index = player->video_stream_index;
    int audio_stream_index = player->audio_stream_index;
    init_codec_context(player, video_stream_index);
    init_codec_context(player, audio_stream_index);

    decode_video_prepare(env, player, surface);
    pthread_create(&(player->decode_threads[video_stream_index]), NULL, decode_data,
                   (void *) player);

}

