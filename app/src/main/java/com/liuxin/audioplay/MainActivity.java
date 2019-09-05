package com.liuxin.audioplay;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.PermissionChecker;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.liuxin.audiolib.LXPlayer;
import com.liuxin.audiolib.OnCompleteListener;
import com.liuxin.audiolib.OnErrorListener;
import com.liuxin.audiolib.OnPlayStatusListener;
import com.liuxin.audiolib.OnPrepareListener;
import com.liuxin.audiolib.OnProgressListener;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {

  private LXPlayer lxPlayer;
  private TextView textView;
  private TextView tv;
  private SeekBar seekBar;
  private int duration;
  private boolean isTouch=false;
  private int volume=100;
  private SeekBar sbVolume;
  private boolean mute=false;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        // Example of a call to a native method
        tv= findViewById(R.id.sample_text);

        textView =findViewById(R.id.textView);
        seekBar=findViewById(R.id.seek);
        sbVolume=findViewById(R.id.seek_volume);
        tv.setText("初始状态");
        sbVolume.setProgress(volume);



        if (PackageManager.PERMISSION_GRANTED!= ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            if (Build.VERSION.SDK_INT >= 23) {
                try {
                    List<String> permissions = new ArrayList<>();
                    if (PackageManager.PERMISSION_GRANTED != PermissionChecker.checkCallingOrSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                        permissions.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
                    }
                    if (permissions.size() != 0) {
                        ActivityCompat.requestPermissions(this,
                                (String[]) permissions.toArray(new String[permissions.size()]),
                                200);

                    }
                } catch (Exception e) {
                    e.printStackTrace();

                }
            }

        }else{
            //已经授权过了
        }


       // tv.setText(demo.stringFromJNI());
        lxPlayer=LXPlayer.getInstance();



        lxPlayer.setOnPlayStatusListener(new OnPlayStatusListener(){

            @Override
            public void playStatus(final int status) {
                if(status==1){
                    tv.setText("播放状态");
                }else if(status==2){
                    tv.setText("暂停状态");
                }else if(status==3){
                    tv.setText("停止状态");
                }else if(status==0){
                    tv.setText("初始状态");
                }



            }
        });
        lxPlayer.setOnPrepareListener(new OnPrepareListener() {
            @Override
            public void prepare() {
                duration=lxPlayer.getDuration();
                lxPlayer.start();

            }
        });

        lxPlayer.setOnProgressListener(new OnProgressListener() {
            @Override
            public void progress(int duration, int current) {

                if(!isTouch){
                    final int min=duration/60;
                    final int sec=duration%60;
                    final int cumin=current/60;
                    final int cusec=current%60;
                    textView.setText(cumin+":"+cusec+"/"+min+":"+sec);
                    if(duration!=0){
                        seekBar.setProgress(current*100/duration);
                    }else {
                        seekBar.setProgress(0);
                    }


                }


            }
        });
        lxPlayer.setOnErrorListener(new OnErrorListener() {
            @Override
            public void error(int code, String msg) {
                Log.e("ffmepg","error code==>"+code+" msg==>"+msg+"thread=>"+Thread.currentThread().getName());
            }
        });
        lxPlayer.setOnCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                 Log.e("ffmpeg","完成");
            }
        });
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                 if(fromUser){
                     final int min=duration/60;
                     final int sec=duration%60;
                     int current=duration*progress/seekBar.getMax();
                     final int cumin=current/60;
                     final int cusec=current%60;
                     textView.setText(cumin+":"+cusec+"/"+min+":"+sec);

                 }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isTouch=true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                   isTouch=false;
                   int currentSec= duration*seekBar.getProgress()/seekBar.getMax();
                   lxPlayer.seek(currentSec);


            }
        });
        sbVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                volume=seekBar.getProgress();
                lxPlayer.setVolume(seekBar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });




    }


    public void CToJava(View view) {
        lxPlayer.prepare("http://od.qingting.fm/m4a/5cfbe4377cb8915f99370adb_12803144_24.m4a");


    }


    public void release(View view) {
         duration=0;
         volume=100;
         sbVolume.setProgress(volume);
        lxPlayer.release();

    }

    public void pause(View view) {
        lxPlayer.pause();
    }

    public void play(View view) {
        lxPlayer.play();
    }

    public void next(View view) {
        duration=0;
        volume=100;
        sbVolume.setProgress(volume);
        lxPlayer.nextOrPre("http://od.qingting.fm/m4a/5a0af44f7cb8914779263740_8222397_24.m4a");
    }


    public void mute(View view) {
        mute=!mute;
        lxPlayer.setMute(mute);
    }

    public void StreoChannel(View view) {
        lxPlayer.setChannelSolo(2);
    }

    public void RightChannel(View view) {
        lxPlayer.setChannelSolo(1);
    }

    public void leftChannel(View view) {
        lxPlayer.setChannelSolo(0);
    }
}
