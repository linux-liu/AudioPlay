//
// Created by liuxin on 19-8-24.
//

#ifndef AUDIOPLAY_AUDIO_H
#define AUDIOPLAY_AUDIO_H
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"


};
#include "AvPacketQuene.h"
#include "PlayStatus.h"
#include "CallJava.h"
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>
#include <unistd.h>
#include "SoundTouch.h"
#include <math.h>
#define OUT_SAMPLE_RATE 44100
#define OUT_CHANNEL_NB 2
#define OUT_FORMAT  AVSampleFormat::AV_SAMPLE_FMT_S16
using namespace soundtouch;
class Audio {
public:
    int audioStreamIndex=-1;

    AVCodecContext *codecContext = NULL;
    AvPacketQuene *avPacketQuene=NULL;
    PlayStatus *playStatus=NULL;
    uint8_t *data=NULL;
    bool  isReadFinish= false;


    int64_t duration=0;
    double current=0;
    AVRational time_base;
    double pre_time=0;


    pthread_t decode_thread;
    CallJava *callJava=NULL;

    //播放相关
    SLObjectItf engineObject=NULL;
    SLEngineItf  slEngineItf=NULL;
    SLAndroidSimpleBufferQueueItf slBufferQueueItf=NULL;


   //混音
    SLObjectItf muixObject=NULL;
    SLEnvironmentalReverbItf  outEnvironmentalReverbItf=NULL;
    SLEnvironmentalReverbSettings slEnvironmentalReverbSettings=SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //播放和播放参数
    SLObjectItf  playerObject=NULL;
    SLPlayItf    slPlayItf=NULL;
    SLVolumeItf  slVolumeItf=NULL;
    SLMuteSoloItf slMuteSoloItf=NULL;

    pthread_mutex_t sl_mutex;

    //soundTouch
    SoundTouch *soundTouch=NULL;
    SAMPLETYPE *sampletype=NULL; //处理后的音频数据
    double pitch=1.0;//变调
    double tempPo=1.0;//变速
    int per_number_sample=0;//每个通道数的采样个数
    bool finished = true; //当前这次采样是否采样完成
    uint8_t *out_buffer = NULL; //临时指针
    int num = 0;
    int data_size=0;

    //db
    time_t  pretime=0;
    time_t  nowtime=0;





public:
    Audio(PlayStatus *status,CallJava *callJava,AvPacketQuene *packetQuene);
    void  resample();

    int start(uint8_t **temp);

    void pause();

    void play();

    void stop();

    void release();

    void setVolume(int volume);

    void setMute(bool mute);

    void setChannelSolo(int channel);


    int dealSoundTouch();

    void  setPitch(double pitch);
    void setTempPo(double temPo);

    int getPCMDB(SAMPLETYPE * bufferData,size_t size);


    void initSLES();

    ~Audio();

};


#endif //AUDIOPLAY_AUDIO_H
