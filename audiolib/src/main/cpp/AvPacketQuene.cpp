//
// Created by liuxin on 19-8-25.
//

#include "AvPacketQuene.h"

AvPacketQuene::AvPacketQuene() {
    pthread_mutex_init(&pthread_mutex,NULL);

    pthread_cond_init(&pthread_cond,NULL);
    queue=new std::queue<AVPacket*>();


}

int AvPacketQuene::pushQueue(AVPacket *avPacket) {
    pthread_mutex_lock(&pthread_mutex);

    if(queue!=NULL)
     queue->push(avPacket);
    if(IS_DEBUG){
   //  ALOGD("入栈,队列的数据中有%ld个包",queue->size());
    }

    pthread_cond_signal(&pthread_cond);
    pthread_mutex_unlock(&pthread_mutex);

    return 0;
}

int AvPacketQuene::pullQueue(AVPacket *avPacket) {

    pthread_mutex_lock(&pthread_mutex);
    if(queue!=NULL&&queue->size()>0){
        AVPacket *pkt=queue->front();
        if(av_packet_ref(avPacket,pkt)==0){
            queue->pop();
        } else{
            return -1;
        }
        av_packet_free(&pkt);
        av_freep(&pkt);
        pkt=NULL;
        if(IS_DEBUG){
          //  ALOGD("出栈,队列的数据中有%ld个包",queue->size());
        }
    } else{
      pthread_cond_wait(&pthread_cond,&pthread_mutex);
    }

    pthread_mutex_unlock(&pthread_mutex);
    return 0;
}

AvPacketQuene::~AvPacketQuene() {
    if(IS_DEBUG){
        ALOGD("AvPacketQuene析构函数调用");
    }

   pthread_mutex_destroy(&pthread_mutex);
   pthread_cond_destroy(&pthread_cond);
}

unsigned int AvPacketQuene::getSize() {
    unsigned int size=0;
    pthread_mutex_lock(&pthread_mutex);

    size=queue->size();
    pthread_mutex_unlock(&pthread_mutex);
    return size;
}

void AvPacketQuene::clearQuene() {
        pthread_cond_signal(&pthread_cond);
        pthread_mutex_lock(&pthread_mutex);
        while (queue!=NULL&&!queue->empty()){
            AVPacket *packet=queue->front();
                queue->pop();
                av_packet_free(&packet);
                av_freep(&packet);

        }
        pthread_mutex_unlock(&pthread_mutex);
}


