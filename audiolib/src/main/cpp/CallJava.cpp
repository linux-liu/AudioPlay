//
// Created by liuxin on 19-8-24.
//

#include "CallJava.h"

CallJava::CallJava(JavaVM *javaVM, JNIEnv *jniEnv, jobject obj) {
    this->jvm=javaVM;
    this->jnv=jniEnv;
    this->jobj=obj;
    jclass jcls=jniEnv->GetObjectClass(jobj);
    this->jmd=jniEnv->GetMethodID(jcls,"JniCallPrepare","()V");
    this->jprogressmd=jniEnv->GetMethodID(jcls,"JniCallProgress","(II)V");
    this->jstatusmd=jniEnv->GetMethodID(jcls,"JniCallPlayStatus","(I)V");
    this->jcompletemd=jniEnv->GetMethodID(jcls,"JniCallComplete","()V");
    this->jerrormd=jniEnv->GetMethodID(jcls,"JniCallError","(ILjava/lang/String;)V");
    this->jdbmd=jniEnv->GetMethodID(jcls,"JniCallDB","(I)V");

}

void CallJava::onPrePare(int type) {
    if(IS_DEBUG){
        ALOGD("onPrePare=>%d",type);
    }
  if(type==ChildThread){
      JNIEnv *jniEnv=NULL;
      jvm->AttachCurrentThread(&jniEnv,NULL);

      jniEnv->CallVoidMethod(jobj,jmd);

      jvm->DetachCurrentThread();
  } else if(type==MainThread){
      jnv->CallVoidMethod(jobj,jmd);
  }
}

CallJava::~CallJava() {

}

void CallJava::onProgress(int type, int duration, int current) {
    if(type==ChildThread){
        JNIEnv *jniEnv=NULL;
        jvm->AttachCurrentThread(&jniEnv,NULL);

        jniEnv->CallVoidMethod(jobj,jprogressmd,duration,current);

        jvm->DetachCurrentThread();
    } else if(type==MainThread){

        jnv->CallVoidMethod(jobj,jprogressmd,duration,current);
    }

}

void CallJava::onPlayStatus(int type, int status) {

    if(type==ChildThread){
        JNIEnv *jniEnv=NULL;
        jvm->AttachCurrentThread(&jniEnv,NULL);

        jniEnv->CallVoidMethod(jobj,jstatusmd,status);

        jvm->DetachCurrentThread();
    } else if(type==MainThread){

        jnv->CallVoidMethod(jobj,jstatusmd,status);
    }
}

void CallJava::onComplete(int type) {
    if(IS_DEBUG){
        ALOGD("onComplete=>%d",type);
    }
    if(type==ChildThread){
        JNIEnv *jniEnv=NULL;
        jvm->AttachCurrentThread(&jniEnv,NULL);

        jniEnv->CallVoidMethod(jobj,jcompletemd);

        jvm->DetachCurrentThread();
    } else if(type==MainThread){

        jnv->CallVoidMethod(jobj,jcompletemd);
    }
}

void CallJava::onError(int type, int code, char *msg) {
    if(IS_DEBUG){
        ALOGD("onError=>%d",code);
    }

    if(type==ChildThread){
        JNIEnv *jniEnv=NULL;
        jvm->AttachCurrentThread(&jniEnv,NULL);

        jstring strMsg=jniEnv->NewStringUTF(msg);

        jniEnv->CallVoidMethod(jobj,jerrormd,code,strMsg);

        jniEnv->DeleteLocalRef(strMsg);

        jvm->DetachCurrentThread();
    } else if(type==MainThread){
        jstring strMsg=jnv->NewStringUTF(msg);
        jnv->CallVoidMethod(jobj,jerrormd,code,strMsg);

        jnv->DeleteLocalRef(strMsg);
    }
}

void CallJava::onDb(int type, int db) {
    if(type==ChildThread){
        JNIEnv *jniEnv=NULL;
        jvm->AttachCurrentThread(&jniEnv,NULL);

        jniEnv->CallVoidMethod(jobj,jdbmd,db);

        jvm->DetachCurrentThread();
    } else if(type==MainThread){

        jnv->CallVoidMethod(jobj,jdbmd,db);
    }
}




