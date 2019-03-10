#include <com_pierce_ffmpeg_MPlayer.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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


JNIEXPORT void JNICALL Java_com_pierce_ffmpeg_MPlayer_render
        (JNIEnv *env, jobject jobj, jstring input_jstr, jobject surface) {

    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGI("%s", "failed to open video file");
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "get video infor failed");
        return;
    }
    int video_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if (pCodeCtx == NULL) {
        LOGE("%s", "failed to decode");
        return;
    }

    if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        LOGE("%s", "failed to open coder");
        return;
    }

    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();

    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_Buffer outbuffer;

    int len, got_frame, framecount = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        len = avcodec_decode_video2(pFormatCtx, yuv_frame, &got_frame, packet);

        if (got_frame) {
            LOGI("decoding->%d frame", framecount++);
        }
        ANativeWindow_setBuffersGeometry(nativeWindow, pCodeCtx->width, pCodeCtx->height,
                                         WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_lock(nativeWindow, &outbuffer, NULL);
        avpicture_fill((AVPicture *) rgb_frame, outbuffer.bits, AV_PIX_FMT_RGBA, pCodeCtx->width,
                       pCodeCtx->height);

        I420ToARGB(yuv_frame->data[0], yuv_frame->linesize[0],
                   yuv_frame->data[2], yuv_frame->linesize[2],
                   yuv_frame->data[1], yuv_frame->linesize[1],
                   rgb_frame->data[0], rgb_frame->linesize[0],
                   pCodeCtx->width, pCodeCtx->height);
        ANativeWindow_unlockAndPost(nativeWindow);
        usleep(1000 * 16);

    }
    av_free_packet(packet);
    ANativeWindow_release(nativeWindow);
    av_frame_free(&yuv_frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
}