#include <com_pierce_ffmpeg_MPlayer.h>
#include <stdlib.h>
#include <unistd.h>
#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRME_SIZE 48000 * 4

//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"

JNIEXPORT void JNICALL Java_com_pierce_ffmpeg_MPlayer_sound
        (JNIEnv *env, jobject jthiz, jstring input_jstr, jstring output_jstr) {

    const char *input_cstr = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    const char *output_cstr = (*env)->GetStringUTFChars(env, output_jstr, NULL);

    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGI("%s", "failed to open audio file");
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGI("%s", "got audio infor failed");
        return;
    }
    int i = 0, audio_stream_idx = -1;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            audio_stream_idx = i;
            break;
        }

    }

    AVCodecContext *codecContext = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecContext->codec_id);
    if (codec == NULL) {
        LOGI("%s", "codec failed");
        return;
    }
    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        LOGI("%s", "open codec failed");
        return;

    }
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    SwrContext *swrContext = swr_alloc();
    enum AVSampleFormat in_sample_fmt = codecContext->sample_fmt;
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int in_sample_rate = codecContext->sample_rate;
    int out_sample_rate = in_sample_rate;
    uint64_t in_ch_layout = codecContext->channel_layout;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout,
                       in_sample_fmt, in_sample_rate, 0, NULL);

    swr_init(swrContext);

    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    jclass player_class = (*env)->GetObjectClass(env, jthiz);
    jmethodID create_audio_track_mid = (*env)->GetStaticMethodID(env, player_class,
                                                                 "createAudioTrack",
                                                                 "(II)Landroid/media/AudioTrack;");
    jobject audio_track = (*env)->CallObjectMethod(env, jthiz, create_audio_track_mid,
                                                   out_sample_rate, out_channel_nb);
    jclass audio_track_class = (*env)->GetObjectClass(env, audio_track);
    jmethodID audio_track_play_mid = (*env)->GetMethodID(env, audio_track_class, "play", "()V");
    (*env)->CallVoidMethod(env, audio_track, audio_track_play_mid);
    jmethodID audio_track_write_mid = (*env)->GetMethodID(env, audio_track_class, "write",
                                                          "([BII)I");
    FILE *fp_pcm = fopen(output_cstr, "wb");
    uint64_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);
    int got_frame = 0, index = 0, ret;

    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audio_stream_idx) {
            ret = avcodec_decode_audio4(codecContext, frame, &got_frame, packet);

            if (ret < 0) {
                LOGI("%s", "decode finished");
            }

            if (got_frame > 0) {
                LOGI("decode：%d", index++);

                swr_convert(swrContext, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) frame->data, frame->nb_samples);
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                                 frame->nb_samples, out_sample_fmt,
                                                                 1);
                fwrite(out_buffer, 1, out_sample_fmt, fp_pcm);
                jbyteArray audio_sample_array = (*env)->NewByteArray(env, out_buffer_size);
                jbyte *sample_bytep = (*env)->GetByteArrayElements(env, audio_sample_array, NULL);
                memcpy(sample_bytep, out_buffer, out_buffer_size);
                (*env)->ReleaseByteArrayElements(env, audio_sample_array, sample_bytep, 0);
                (*env)->CallIntMethod(env, audio_track, audio_track_write_mid, audio_sample_array,
                                      0, out_buffer_size);
                (*env)->DeleteGlobalRef(env, audio_sample_array);
                usleep(1000 * 16);
            }

        }
        av_free_packet(packet);

    }

    av_frame_free(&frame);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&pFormatCtx);
    (*env)->ReleaseStringUTFChars(env, input_jstr, input_cstr);
    (*env)->ReleaseStringUTFChars(env, output_jstr, output_cstr);

}