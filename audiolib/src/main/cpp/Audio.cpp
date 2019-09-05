//
// Created by liuxin on 19-8-24.
//


#include "Audio.h"

Audio::Audio(PlayStatus *status, CallJava *callJava1, AvPacketQuene *packetQuene) {
    this->playStatus = status;
    this->callJava = callJava1;
    this->avPacketQuene = packetQuene;
    current = 0;
    pre_time = 0;
    data = (uint8_t *) av_malloc(OUT_CHANNEL_NB * 2 * OUT_SAMPLE_RATE);
    pthread_mutex_init(&sl_mutex, NULL);

}

static void *decode_thread_call_back(void *data) {
    Audio *audio = (Audio *) data;
    audio->initSLES();
    pthread_exit(&audio->decode_thread);
}

void Audio::resample() {
    pthread_create(&decode_thread, NULL, decode_thread_call_back, this);
}


int Audio::start() {
    int readBufferSize = 0;
    while (playStatus != NULL && !playStatus->isExit) {
        unsigned int size = avPacketQuene->getSize();
        int ret = 0;
        if (size > 0) {
            //  ALOGD("有数据了");
            AVPacket *avPacket = av_packet_alloc();
            avPacketQuene->pullQueue(avPacket);
            ret = avcodec_send_packet(codecContext, avPacket);
            if (ret < 0) {
                av_packet_free(&avPacket);
                av_freep(&avPacket);
                avPacket = NULL;
                continue;
            }
            AVFrame *avFrame = av_frame_alloc();
            avFrame->channels = codecContext->channels;
            avFrame->channel_layout = codecContext->channel_layout;
            avFrame->format = codecContext->sample_fmt;
            avFrame->sample_rate = codecContext->sample_rate;
            avFrame->nb_samples = codecContext->frame_size;
            ret = avcodec_receive_frame(codecContext, avFrame);
            if (ret == 0) {
                if (avFrame->channels <= 0 && avFrame->channel_layout > 0) {
                    avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);

                }
                if (avFrame->channel_layout <= 0 && avFrame->channels > 0) {
                    avFrame->channel_layout = (uint64_t) (av_get_default_channel_layout(
                            avFrame->channels));
                }


                SwrContext *swrContext = swr_alloc_set_opts(NULL, av_get_default_channel_layout(
                        OUT_CHANNEL_NB), OUT_FORMAT, OUT_SAMPLE_RATE, avFrame->channel_layout,

                                                            (AVSampleFormat) (avFrame->format),
                                                            avFrame->sample_rate, 0, NULL);

                if (swrContext == NULL || swr_init(swrContext) < 0) {
                    av_packet_free(&avPacket);
                    av_freep(&avPacket);
                    avPacket = NULL;

                    av_frame_free(&avFrame);
                    av_freep(&avFrame);
                    avFrame = NULL;
                    if (swrContext != NULL) {
                        swr_free(&swrContext);
                    }
                    continue;

                }


                int per_number_sample = swr_convert(swrContext, &data, avFrame->nb_samples,
                                                    (const uint8_t **) avFrame->data,
                                                    avFrame->nb_samples);

                int frame_size_by_byte =
                        per_number_sample * OUT_CHANNEL_NB * av_get_bytes_per_sample(OUT_FORMAT);

                readBufferSize = frame_size_by_byte;

                current = avFrame->pts * av_q2d(time_base);

                av_packet_free(&avPacket);
                av_freep(&avPacket);
                avPacket = NULL;

                av_frame_free(&avFrame);
                av_freep(&avFrame);
                avFrame = NULL;
                if (swrContext != NULL) {
                    swr_free(&swrContext);
                }

                break;


            } else {
                av_packet_free(&avPacket);
                av_freep(&avPacket);
                avPacket = NULL;

                av_frame_free(&avFrame);
                av_freep(&avFrame);
                avFrame = NULL;
                continue;
            }


        } else {
            if (IS_DEBUG) {
                ALOGD("没有数据可以采样");

            }
            if (isReadFinish) {
                if (callJava != NULL) {
                    callJava->onComplete(ChildThread);
                }
            }

        }
    }
    return readBufferSize;
}


static void bufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *data) {
    Audio *audio = (Audio *) data;
    int buffsize = audio->start();
    double show_time = buffsize / (double) (OUT_SAMPLE_RATE * OUT_CHANNEL_NB * 2);

    audio->current = audio->current + show_time;

    if (audio->current - audio->pre_time > 0.5) {
        audio->callJava->onProgress(ChildThread, (int) audio->duration, (int) audio->current);
        audio->pre_time = audio->current;
    }

    if (audio->data && buffsize > 0)
        (*(audio->slBufferQueueItf))->Enqueue(audio->slBufferQueueItf, audio->data, buffsize);
}

void Audio::initSLES() {
    pthread_mutex_lock(&sl_mutex);
    SLresult result;
    //创建了引擎
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    (void) result;
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    (void) result;
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &slEngineItf);

    (void) result;

    //创建混音
    const SLInterfaceID sids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean slb[1] = {SL_BOOLEAN_FALSE};
    result = (*slEngineItf)->CreateOutputMix(slEngineItf, &muixObject, 1, sids, slb);

    (void) result;
    result = (*muixObject)->Realize(muixObject, SL_BOOLEAN_FALSE);

    (void) result;
    result = (*muixObject)->GetInterface(muixObject, SL_IID_ENVIRONMENTALREVERB,
                                         &outEnvironmentalReverbItf);
    //  ALOGE("result==>%d",result);
    if (result == SL_RESULT_SUCCESS) {
        result = (*outEnvironmentalReverbItf)->SetEnvironmentalReverbProperties(
                outEnvironmentalReverbItf, &slEnvironmentalReverbSettings);

        //  ALOGE("result==>%d",result);
        (void) result;
    }




    //设置播放器参数和创建播放器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, muixObject};
    SLDataSink audioSnk = {&outputMix, NULL};

    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            SL_SAMPLINGRATE_44_1,//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME,SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    result = (*slEngineItf)->CreateAudioPlayer(slEngineItf, &playerObject, &slDataSource, &audioSnk,
                                               3, ids, req);
    //初始化播放器
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &slPlayItf);
//    注册回调缓冲区 获取缓冲队列接口
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &slBufferQueueItf);

    //获取音量接口
    (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &slVolumeItf);

    //声道切换
    (*playerObject)->GetInterface(playerObject,SL_IID_MUTESOLO,&slMuteSoloItf);


    //缓冲接口回调
    (*slBufferQueueItf)->RegisterCallback(slBufferQueueItf, bufferCallBack, this);

//    获取播放状态接口
    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);

//    主动调用回调函数开始工作
    bufferCallBack(slBufferQueueItf, this);
    if (playStatus != NULL) {
        playStatus->playStatus = STATUS_PLAYING;
    }
    if (callJava != NULL) {
        callJava->onPlayStatus(ChildThread, STATUS_PLAYING);
    }
    pthread_mutex_unlock(&sl_mutex);

}

void Audio::pause() {
    if (playStatus != NULL && playStatus->playStatus != STATUS_PAUSE) {

        if (engineObject != NULL && playerObject != NULL && slPlayItf != NULL) {
            ALOGD("pause");
            (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PAUSED);
            playStatus->playStatus = STATUS_PAUSE;
            if (callJava != NULL) {
                callJava->onPlayStatus(MainThread, STATUS_PAUSE);
            }
        }
    }


}

void Audio::play() {
    if (playStatus != NULL && playStatus->playStatus != STATUS_PLAYING) {
        if (engineObject != NULL && playerObject != NULL && slPlayItf != NULL) {
            (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
            playStatus->playStatus = STATUS_PLAYING;
            ALOGD("playing");
            if (callJava != NULL) {
                callJava->onPlayStatus(MainThread, STATUS_PLAYING);
            }
        }
    }


}

void Audio::stop() {
    if (playStatus != NULL && playStatus->playStatus != STAUTS_STOP) {
        if (engineObject != NULL && playerObject != NULL && slPlayItf != NULL) {
            (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_STOPPED);
            playStatus->playStatus = STAUTS_STOP;
            ALOGD("stop");
            if (callJava != NULL) {
                callJava->onPlayStatus(ChildThread, STAUTS_STOP);
            }
        }

    }

}

void Audio::setVolume(int percent) {
    if (slVolumeItf != NULL) {
        int millibel = 0;
        if (percent > 30) {

            millibel = (100 - percent) * -20;
        } else if (percent > 25) {
            millibel = (100 - percent) * -22;
        } else if (percent > 20) {
            millibel = (100 - percent) * -25;
        } else if (percent > 15) {
            millibel = (100 - percent) * -28;
        } else if (percent > 10) {
            millibel = (100 - percent) * -30;
        } else if (percent > 5) {
            millibel = (100 - percent) * -34;
        } else if (percent > 3) {
            millibel = (100 - percent) * -37;
        } else if (percent > 0) {
            millibel = (100 - percent) * -40;
        } else {
            millibel = (100 - percent) * -100;
        }

        (*slVolumeItf)->SetVolumeLevel(slVolumeItf, millibel);
    }

}
void Audio::setMute(bool mute) {
    if (slVolumeItf != NULL) {
        (*slVolumeItf)->SetMute(slVolumeItf,mute);
    }
}

void Audio::setChannelSolo(int channel) {
   if(slMuteSoloItf!=NULL){
       if(channel==0){
           (*slMuteSoloItf)->SetChannelSolo(slMuteSoloItf,0,true);
           (*slMuteSoloItf)->SetChannelSolo(slMuteSoloItf,1,false);
       } else if(channel==1){
           (*slMuteSoloItf)->SetChannelSolo(slMuteSoloItf,0,false);
           (*slMuteSoloItf)->SetChannelSolo(slMuteSoloItf,1,true);
       } else{
           (*slMuteSoloItf)->SetChannelSolo(slMuteSoloItf,0,true);
           (*slMuteSoloItf)->SetChannelSolo(slMuteSoloItf,1,true);
       }

   }
}


void Audio::release() {
    int try_count = 100000;
    int ret = -1;

    while (try_count) {
        ret = pthread_mutex_trylock(&sl_mutex);
        if (ret == 0) {
            break;
        }
        try_count--;
        usleep(1000 * 10);
    }

    stop();
    if (playStatus != NULL) {
        playStatus->isExit = true;
        playStatus = NULL;
    }
    callJava = NULL;

    if (avPacketQuene) {
        avPacketQuene->clearQuene();
    }
    if (playerObject) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        slPlayItf = NULL;
        slBufferQueueItf = NULL;
        slVolumeItf=NULL;
        slMuteSoloItf=NULL;
    }

    if (muixObject) {
        (*muixObject)->Destroy(muixObject);
        muixObject = NULL;
        outEnvironmentalReverbItf = NULL;


    }

    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        slEngineItf = NULL;
    }

    pthread_mutex_unlock(&sl_mutex);

    if (data) {
        av_freep(&data);
        data = NULL;
    }


}

Audio::~Audio() {
    if (IS_DEBUG) {
        ALOGD("Audio析构函数开始执行");
    }
    pthread_mutex_destroy(&sl_mutex);

}











