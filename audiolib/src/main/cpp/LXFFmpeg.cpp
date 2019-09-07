//
// Created by liuxin on 19-8-24.
//


#include "LXFFmpeg.h"


void *decodeThread(void *data) {
    LXFFmpeg *lxfFmpeg = (LXFFmpeg *) data;
    lxfFmpeg->realPrepare();
    pthread_exit(&lxfFmpeg->dthread);

}

LXFFmpeg::LXFFmpeg(PlayStatus *status, const char *url, CallJava *callJava) {
    memcpy(this->url, url, 1000);
    this->callJava = callJava;
    this->playStatus = status;
    this->avPacketQuene = new AvPacketQuene();
    pthread_mutex_init(&pthreadMutex, NULL);
    pthread_mutex_init(&pthreadSeek, NULL);

}


void LXFFmpeg::prepare() {

    pthread_create(&dthread, NULL, decodeThread, this);
}

static int interrupt_call_back(void *data) {
    LXFFmpeg *lxfFmpeg = (LXFFmpeg *) data;

    if (lxfFmpeg != NULL && lxfFmpeg->playStatus != NULL) {
        if (lxfFmpeg->playStatus->isExit) {
            return AVERROR_EOF;
        } else {
            return 0;
        }
    } else {
        return AVERROR_EOF;
    }

}


void LXFFmpeg::realPrepare() {

    pthread_mutex_lock(&pthreadMutex);
    callJava->onPlayStatus(ChildThread, 0);
    av_register_all();
    avformat_network_init();
    if (this->audio == NULL) {
        this->audio = new Audio(playStatus, callJava, avPacketQuene);
    }
    formatContext = avformat_alloc_context();
    formatContext->interrupt_callback.callback = interrupt_call_back;
    formatContext->interrupt_callback.opaque = this;
    int ret = 0;
    if ((ret = avformat_open_input(&formatContext, url, NULL, NULL)) < 0) {
        if (IS_DEBUG) {
            ALOGE("打开输入流失败%s", av_err2str(ret));
        }

        callJava->onError(ChildThread, ERROR_IO, av_err2str(ret));

        pthread_mutex_unlock(&pthreadMutex);
        return;
    }


    if ((ret = avformat_find_stream_info(formatContext, NULL)) < 0) {
        if (IS_DEBUG) {
            ALOGE("找到流信息失败");

        }
        callJava->onError(ChildThread, ERROR_IO_INFO, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }

    ret = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        if (IS_DEBUG) {
            ALOGE("找到音频流失败");
        }
        callJava->onError(ChildThread, ERROR_FIND_STREAM_FAIL, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }

    audio->audioStreamIndex = ret;

    codecContext = avcodec_alloc_context3(NULL);
    if (!codecContext) {
        if (IS_DEBUG) {
            ALOGE("分配编码上下文失败");

        }
        callJava->onError(ChildThread, ERROR_CONTEXT_MALLOC_FAIL,
                          "failed to malloc avcodec context");
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }


    this->audio->duration = (int64_t) (formatContext->duration / AV_TIME_BASE);
    if (callJava != NULL) {
        callJava->onProgress(ChildThread, (int) this->audio->duration, 0);
    }
    if ((ret = avcodec_parameters_to_context(codecContext,
                                             formatContext->streams[audio->audioStreamIndex]->codecpar)) <
        0) {
        if (IS_DEBUG) {
            ALOGE("拷贝参数失败");

        }
        callJava->onError(ChildThread, ERROR_COPY_PARM_FAIL, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }

    audio->codecContext = codecContext;
    audio->time_base = formatContext->streams[audio->audioStreamIndex]->time_base;
    AVCodec *avCodec = avcodec_find_decoder(
            formatContext->streams[audio->audioStreamIndex]->codecpar->codec_id);
    if (!avCodec) {
        if (IS_DEBUG) {
            ALOGE("找到解码器失败");
        }

        callJava->onError(ChildThread, ERROR_FIND_DECODE_FAIL, "find decoder error");
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }
    if ((ret = avcodec_open2(codecContext, avCodec, NULL)) < 0) {
        if (IS_DEBUG) {
            ALOGE("打开解码器失败");
        }
        callJava->onError(ChildThread, ERROR_OPEN_DECODE_FAIL, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }
    pthread_mutex_unlock(&pthreadMutex);

    if (callJava) {
        callJava->onPrePare(ChildThread);
    }

}


static void *startCallBack(void *data) {
    LXFFmpeg *lxfFmpeg = (LXFFmpeg *) data;
    lxfFmpeg->realStart();
    pthread_exit(&lxfFmpeg->dStartThread);
}


void LXFFmpeg::start() {
    if (audio == NULL || playStatus == NULL || playStatus->isExit) {
        if (IS_DEBUG) {
            ALOGE("请先调用prepare方法准备音频流");
        }
        return;
    }

    audio->resample();
    pthread_create(&dStartThread, NULL, startCallBack, this);


}

void LXFFmpeg::realStart() {


    pthread_mutex_lock(&pthreadMutex);
    while (playStatus != NULL && !playStatus->isExit && avPacketQuene != NULL) {

        if (playStatus->isSeek) {
            continue;
        }
        //最多缓存100个
        if (avPacketQuene->getSize() > 100) {
            continue;
        }


        AVPacket *avPacket = av_packet_alloc();
        pthread_mutex_lock(&pthreadSeek);

        int ret = av_read_frame(formatContext, avPacket);
        pthread_mutex_unlock(&pthreadSeek);
        if (ret == 0) {
            if (avPacket->stream_index == audio->audioStreamIndex) {

                avPacketQuene->pushQueue(avPacket);
            } else {
                if (avPacket) {
                    av_packet_free(&avPacket);
                    av_freep(&avPacket);
                    avPacket = NULL;
                }
            }


        } else {
            if (avPacket) {
                av_packet_free(&avPacket);
                av_freep(&avPacket);
                avPacket = NULL;
            }
            ALOGE("读取完成！");
            break;
        }

    }

    if (audio != NULL) {
        audio->isReadFinish = true;
    }

    pthread_mutex_unlock(&pthreadMutex);


}


LXFFmpeg::~LXFFmpeg() {
    pthread_mutex_destroy(&pthreadMutex);
    pthread_mutex_destroy(&pthreadSeek);


}


void LXFFmpeg::play() {
    if (audio != NULL) {
        audio->play();
    }
}

void LXFFmpeg::pause() {
    if (audio != NULL) {
        audio->pause();
    }
}

void LXFFmpeg::release() {
    if (IS_DEBUG) {
        ALOGD("开始释放资源了");
    }


    playStatus->isExit = true;


    int try_count = 100000;
    int ret = -1;

    while (try_count) {
        ret = pthread_mutex_trylock(&pthreadMutex);
        if (ret == 0) {
            ALOGD("获取到锁11");
            break;
        }
        try_count--;
        usleep(1000 * 10);
    }

    ALOGD("开始释放资源了11");

    if (audio != NULL) {
        audio->release();
        delete audio;
        audio = NULL;
    }
    if (callJava != NULL) {
        callJava->onProgress(ChildThread, 0, 0);
    }
    callJava = NULL;
    playStatus = NULL;

    if (avPacketQuene) {
        delete avPacketQuene;
        avPacketQuene = NULL;
    }

    if (codecContext) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        av_freep(&codecContext);
    }

    if (formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        av_freep(&formatContext);
        formatContext = NULL;
    }
    avformat_network_deinit();


    pthread_mutex_unlock(&pthreadMutex);


}

void LXFFmpeg::seek(int sec) {


    if (audio != NULL && avPacketQuene != NULL && playStatus != NULL) {

        if (audio->duration < 0) {
            return;
        }

        if (sec < 0 || sec > audio->duration) {
            return;
        }

        pthread_mutex_lock(&pthreadSeek);
        playStatus->isSeek = true;
        avPacketQuene->clearQuene();
        audio->current = 0;
        audio->pre_time = 0;

        int64_t realSeek = sec * AV_TIME_BASE;
        if (IS_DEBUG) {
            ALOGD("sec=>%d realSeek=>%lld", sec, realSeek);
        }
        int ret = avformat_seek_file(formatContext, -1, INT64_MIN, realSeek, INT64_MAX, 0);
        if (IS_DEBUG) {
            ALOGD("seek ret==%d", ret);
        }

        playStatus->isSeek = false;

        pthread_mutex_unlock(&pthreadSeek);
    }


}

int LXFFmpeg::getDuration() {
    if (audio != NULL) {
        return (int) audio->duration;
    }
    return 0;
}

void LXFFmpeg::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    if (audio != NULL) {
        audio->setVolume(volume);
    }
}

void LXFFmpeg::setMute(bool mute) {
    if (audio != NULL) {
        audio->setMute(mute);
    }

}

void LXFFmpeg::setChannelSolo(int channel) {
    if (audio != NULL) {
        audio->setChannelSolo(channel);
    }
}

void LXFFmpeg::setPitch(double pitch) {
    if(audio!=NULL){
        audio->setPitch(pitch);
    }

}

void LXFFmpeg::setTempPo(double temPo) {
    if(audio!=NULL){
        audio->setTempPo(temPo);
    }
}






