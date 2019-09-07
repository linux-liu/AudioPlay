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
    JavaVM *jvm=NULL;
    JNIEnv *jnv=NULL;
    jobject jobj=NULL;
    jmethodID jmd=NULL;

    jmethodID jstatusmd=NULL;

    jmethodID jprogressmd=NULL;

    jmethodID jcompletemd=NULL;

    jmethodID jerrormd=NULL;

    jmethodID  jdbmd=NULL;






public:
    CallJava(JavaVM *javaVM,JNIEnv *jniEnv,jobject jobj);

    void onPrePare(int type);
    void onProgress(int type,int duration,int current);

    void onComplete(int type);

    void onError(int type,int code,char *msg);

    void onPlayStatus(int type,int status);

    void onCallSwitch(int type);

    void onDb(int type,int db);

    ~CallJava();

};


#endif //AUDIOPLAY_CALLJAVA_H
