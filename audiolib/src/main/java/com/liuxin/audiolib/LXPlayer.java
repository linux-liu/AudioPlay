package com.liuxin.audiolib;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;


public class LXPlayer {


    private volatile boolean isSwitch=false;

    private OnPrepareListener onPrepareListener;

    private OnProgressListener onProgressListener;

    private OnPlayStatusListener onPlayStatusListener;

    private OnCompleteListener onCompleteListener;

    private OnErrorListener onErrorListener;

    private static LXPlayer lxPlayer;

    private String url;

    private  final int MSG_PREPARE=0x123;
    private final int MSG_PROGRESS=0x124;
    private final int MSG_PLAYSTATUS=0x125;
    private final int MSG_COMPLETE=0x126;
    private final int MSG_ERROR=0x127;
    private final int MSG_CALL_SWITCH=0x128;



    private Handler mHandler=new Handler(Looper.getMainLooper()){

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what){
                case MSG_PREPARE:
                    if(onPrepareListener!=null){
                        onPrepareListener.prepare();
                    }
                    break;
                case MSG_PROGRESS:
                    if(onProgressListener!=null){
                        onProgressListener.progress(msg.arg1,msg.arg2);
                    }
                    break;
                case MSG_PLAYSTATUS:
                    if(onPlayStatusListener!=null){
                        onPlayStatusListener.playStatus(msg.arg1);
                    }
                    break;
                case MSG_COMPLETE:
                    release();
                    if(onCompleteListener!=null){
                        onCompleteListener.onComplete();
                    }
                    break;
                case MSG_ERROR:
                    release();
                    if(onErrorListener!=null){
                        onErrorListener.error(msg.arg1,(String)msg.obj);
                    }
                    break;

                case MSG_CALL_SWITCH:
                    if(isSwitch){
                        isSwitch=false;
                        prepare(url);
                    }
                    break;
            }
        }
    };




    private LXPlayer(){

    }

    public static LXPlayer getInstance(){
        if(lxPlayer==null){
            lxPlayer=new LXPlayer();
        }
        return lxPlayer;
    }


    public void setOnPrepareListener(OnPrepareListener listener){
        this.onPrepareListener=listener;
    }

    public void setOnProgressListener(OnProgressListener listener){
        this.onProgressListener=listener;
    }

    public void setOnPlayStatusListener(OnPlayStatusListener listener){
        this.onPlayStatusListener=listener;
    }

    public void setOnCompleteListener(OnCompleteListener listener){
        this.onCompleteListener=listener;
    }

    public void setOnErrorListener(OnErrorListener listener){
        this.onErrorListener=listener;
    }





    static {
        System.loadLibrary("native-lib");

    }


    /**
     * jni调用准备好了的方法
     */
   private void JniCallPrepare(){
       Message message=Message.obtain();
       message.what=MSG_PREPARE;
       mHandler.sendMessage(message);

    }


    private void JniCallProgress(int duration,int current){
        Message message=Message.obtain();
        message.what=MSG_PROGRESS;
        message.arg1=duration;
        message.arg2=current;
        mHandler.sendMessage(message);


    }

    private void JniCallPlayStatus(int status){
        Message message=Message.obtain();
        message.what=MSG_PLAYSTATUS;
        message.arg1=status;

        mHandler.sendMessage(message);

    }

    private void JniCallComplete(){
        Message message=Message.obtain();
        message.what=MSG_COMPLETE;
        mHandler.sendMessage(message);
    }

    private void JniCallError(int code,String msg){

        Message message=Message.obtain();
        message.what=MSG_ERROR;
        message.arg1=code;
        message.obj=msg;
        mHandler.sendMessage(message);

    }

    private void JniCallSwitch(){

        Message message=Message.obtain();
        message.what=MSG_CALL_SWITCH;
        mHandler.sendMessage(message);

    }


    /**
     * 所以公有方法在主线程中调用
     * @param url
     */
    public void nextOrPre(String url){
        if(TextUtils.isEmpty(url)) return;
        this.url=url;
        isSwitch=true;

        release();

    }

    public native void  prepare(String url);

    public native void start();

    public native void pause();

    public native void play();

    public native void seek(int sec);

    public native void release();

    public native int getDuration();

    /**
     * 0到100 0最小 100最大
     * @param volume
     */
    public native void setVolume(int volume);

    public native void setMute(boolean mute);

    /**
     *
     * @param channel  0左声道 1右声道 2立体声
     */
    public native void setChannelSolo(int channel);


}
