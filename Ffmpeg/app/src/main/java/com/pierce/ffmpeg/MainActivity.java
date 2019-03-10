package com.pierce.ffmpeg;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Surface;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.File;

public class MainActivity extends AppCompatActivity {

    private VideoView videoView;
    private Spinner sp_video;
    private MPlayer mPlayer;

    private Button button;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        button = findViewById(R.id.button);
        videoView = (VideoView) findViewById(R.id.video_view);
        sp_video = (Spinner) findViewById(R.id.sp_video);
        String[] videoArray = getResources().getStringArray(R.array.video_list);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1,
                android.R.id.text1, videoArray);
        sp_video.setAdapter(adapter);
        mPlayer=new MPlayer();
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mPlay();
            }
        });

    }

    public void mPlay() {
        String video = sp_video.getSelectedItem().toString();
        String input = new File(Environment.getExternalStorageDirectory(), video).getAbsolutePath();
        //Surface传入到Native函数中，用于绘制
        Surface surface = videoView.getHolder().getSurface();
        //player.render(input, surface);

        //String input = new File(Environment.getExternalStorageDirectory(),"hehuoren.flv").getAbsolutePath();
        //String output = new File(Environment.getExternalStorageDirectory(),"Justin Bieber - Boyfriend.pcm").getAbsolutePath();
        //player.sound(input, output);

        mPlayer.play(input, surface);
    }
}
