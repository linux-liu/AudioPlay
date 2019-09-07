//
// Created by liuxin on 19-8-24.
//

#ifndef AUDIOPLAY_LXFFMPEG_H
#define AUDIOPLAY_LXFFMPEG_H


#include "CallJava.h"
#include "mylog.h"
#include "Audio.h"
#include <pthread.h>
#include "AvPacketQuene.h"
#include "PlayStatus.h"
#include <unistd.h>



extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

class LXFFmpeg {

public:
    CallJava *callJava = NULL;
    PlayStatus *playStatus=NULL;
    Audio *audio = NULL;
    AVFormatContext *formatContext=NULL;
    AVCodecContext *codecContext=NULL;
    pthread_t dthread;
    pthread_mutex_t pthreadMutex;
    AvPacketQuene *avPacketQuene=NULL;

    pthread_mutex_t pthreadSeek;

    pthread_t dStartThread;

     char url[1000];



public:
    LXFFmpeg(PlayStatus *status,const char *url, CallJava *callJava);

    void prepare();

    void start();

    void realPrepare();

    void realStart();



    void play();

    void pause();

    void seek(int sec);

    void release();

    int getDuration();

    void setVolume(int volume);

    void setMute(bool mute);

    void setChannelSolo(int channel);

    void  setPitch(double pitch);
    void setTempPo(double temPo);



    ~LXFFmpeg();
};




#endif //AUDIOPLAY_LXFFMPEG_H
