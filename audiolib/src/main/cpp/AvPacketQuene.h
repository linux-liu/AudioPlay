//
// Created by liuxin on 19-8-25.
//

#ifndef AUDIOPLAY_AVPACKETQUENE_H
#define AUDIOPLAY_AVPACKETQUENE_H

#include<queue>
#include <pthread.h>
#include "mylog.h"
extern "C"{
#include <libavcodec/avcodec.h>
};
class AvPacketQuene {

public:
    std::queue<AVPacket *> *queue=NULL;
    pthread_mutex_t pthread_mutex;
    pthread_cond_t  pthread_cond;
public:
    AvPacketQuene();
    ~AvPacketQuene();
    int pushQueue(AVPacket *avPacket);
    int pullQueue(AVPacket *avPacket);

    void clearQuene();
    unsigned int getSize();


};


#endif //AUDIOPLAY_AVPACKETQUENE_H
