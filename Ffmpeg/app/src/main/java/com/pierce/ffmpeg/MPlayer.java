package com.pierce.ffmpeg;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.Surface;

public class MPlayer {

    static {
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("myffmpeg");
    }


    public native void render(String input, Surface surface);

    public native void sound(String input, String output);

    public native void play(String input, Surface surface);

    public AudioTrack createAudiotrack(int sampleRate, int channel) {
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        int chanelConfig;
        if (channel == 1) {
            chanelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (channel == 2) {
            chanelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            chanelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRate, chanelConfig, audioFormat);
        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, chanelConfig, audioFormat, bufferSizeInBytes, AudioTrack.MODE_STREAM);

        return audioTrack;


    }

}
