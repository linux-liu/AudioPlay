#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include "mylog.h"
#include "LXFFmpeg.h"


JavaVM *jvm = NULL;
CallJava *callJava = NULL;
LXFFmpeg *lxfFmpeg = NULL;
PlayStatus *playStatus = NULL;
bool isexit = true;
pthread_t releaseThread;
pthread_t seekThread;
jobject globelobject = NULL;
int seekSec = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    jvm = javaVM;


    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    if (globelobject != NULL) {
        JNIEnv *jniEnv = NULL;
        vm->GetEnv((void **) &jniEnv, JNI_VERSION_1_6);
        if (jniEnv != NULL) {
            if (IS_DEBUG) {
                ALOGD("释放资源 JNI_OnUnload");
            }

            jniEnv->DeleteGlobalRef(globelobject);
            globelobject = NULL;
        }
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_audiolib_LXPlayer_prepare(JNIEnv *env, jobject instance, jstring url) {


    if (lxfFmpeg == NULL && isexit) {
        const char *_url = env->GetStringUTFChars(url, NULL);
        if (globelobject == NULL) {
            globelobject = env->NewGlobalRef(instance);
        }
        if(IS_DEBUG){
            ALOGD("开始初始化....");
        }
        if (callJava == NULL) {
            callJava = new CallJava(jvm, env, globelobject);
        }
        if (playStatus == NULL) {
            playStatus = new PlayStatus();
        }
        lxfFmpeg = new LXFFmpeg(playStatus, _url, callJava);

        lxfFmpeg->prepare();
        env->ReleaseStringUTFChars(url, _url);
    }



}extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_audiolib_LXPlayer_start(JNIEnv *env, jobject instance) {
    if (lxfFmpeg != NULL) {
        lxfFmpeg->start();
    }


}







extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_audiolib_LXPlayer_play(JNIEnv *env, jobject thiz) {
    if (lxfFmpeg) {
        lxfFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_audiolib_LXPlayer_pause(JNIEnv *env, jobject thiz) {
    if (lxfFmpeg) {
        lxfFmpeg->pause();
    }
}


static void *releaseCallBack(void *data) {
    isexit = false;
    if (lxfFmpeg) {

        lxfFmpeg->release();
        delete lxfFmpeg;
        lxfFmpeg = NULL;

        if (playStatus) {
            delete playStatus;
            playStatus = NULL;
        }
        if (callJava) {
            delete callJava;
            callJava = NULL;
        }

    }
    isexit = true;
    JNIEnv *jniEnv = NULL;
    jvm->AttachCurrentThread(&jniEnv, NULL);
    jclass jcls = jniEnv->GetObjectClass(globelobject);
    jmethodID jcallswitchmd = jniEnv->GetMethodID(jcls, "JniCallSwitch", "()V");
    jniEnv->CallVoidMethod(globelobject, jcallswitchmd);
    jvm->DetachCurrentThread();
    if(IS_DEBUG){
        ALOGD("完全释放了内存");
    }

    pthread_exit(&releaseThread);


}


extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_audiolib_LXPlayer_release(JNIEnv *env, jobject thiz) {

    if (!isexit) {
        return;
    }
    if (globelobject == NULL) {
        globelobject = env->NewGlobalRef(thiz);
    }

    pthread_create(&releaseThread, NULL, releaseCallBack, NULL);


}

static void *seekCallBack(void *data) {
    if (lxfFmpeg != NULL) {
        lxfFmpeg->seek(seekSec);
    }
    pthread_exit(&seekThread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_audiolib_LXPlayer_seek(JNIEnv *env, jobject thiz, jint sec) {

    seekSec = sec;
    pthread_create(&seekThread, NULL, seekCallBack, NULL);


}