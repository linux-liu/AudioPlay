//
// Created by liuxin on 19-8-24.
//

#ifndef AUDIOPLAY_CALLJAVA_H
#define AUDIOPLAY_CALLJAVA_H


#include "jni.h"
#include "mylog.h"
#define MainThread 1
#define ChildThread 0

class CallJava {
public:
    JavaVM *jvm;
    JNIEnv *jnv;
    jobject jobj;
    jmethodID jmd;

    jmethodID jstatusmd;

    jmethodID jprogressmd;

    jmethodID jcompletemd;

    jmethodID jerrormd;






public:
    CallJava(JavaVM *javaVM,JNIEnv *jniEnv,jobject jobj);

    void onPrePare(int type);
    void onProgress(int type,int duration,int current);

    void onComplete(int type);

    void onError(int type,int code,char *msg);

    void onPlayStatus(int type,int status);

    void onCallSwitch(int type);

    ~CallJava();

};


#endif //AUDIOPLAY_CALLJAVA_H
