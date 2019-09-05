cmake_minimum_required(VERSION 3.4.1)

find_library(log-lib log)
find_library(android-lib android)


add_library(native-lib
            SHARED
            src/main/cpp/native-lib.cpp
            src/main/cpp/CallJava.cpp
            src/main/cpp/LXFFmpeg.cpp
            src/main/cpp/Audio.cpp
            src/main/cpp/AvPacketQuene.cpp
            src/main/cpp/PlayStatus.cpp
            src/main/cpp/soundtouch/AAFilter.cpp
            src/main/cpp/soundtouch/FIRFilter.cpp
            src/main/cpp/soundtouch/BPMDetect.cpp
            src/main/cpp/soundtouch/PeakFinder.cpp
            src/main/cpp/soundtouch/cpu_detect_x86.cpp
            src/main/cpp/soundtouch/FIFOSampleBuffer.cpp
            src/main/cpp/soundtouch/InterpolateCubic.cpp
            src/main/cpp/soundtouch/InterpolateLinear.cpp
            src/main/cpp/soundtouch/InterpolateShannon.cpp
            src/main/cpp/soundtouch/mmx_optimized.cpp
            src/main/cpp/soundtouch/RateTransposer.cpp
            src/main/cpp/soundtouch/SoundTouch.cpp
            src/main/cpp/soundtouch/sse_optimized.cpp
            src/main/cpp/soundtouch/TDStretch.cpp
           )

set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs)



add_library(avutil
            SHARED
            IMPORTED )
set_target_properties(avutil
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavutil.so )

add_library(swresample
            SHARED
            IMPORTED )
set_target_properties(swresample
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswresample.so )

add_library(swscale
            SHARED
            IMPORTED )
set_target_properties(swscale
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswscale.so )

add_library(avcodec
            SHARED
            IMPORTED )
set_target_properties(avcodec
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavcodec.so )

add_library(avformat
            SHARED
            IMPORTED )
set_target_properties(avformat
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavformat.so )

add_library(avfilter
            SHARED
            IMPORTED )
set_target_properties(avfilter
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavfilter.so )

add_library(avdevice
            SHARED
            IMPORTED )
set_target_properties(avdevice
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavdevice.so )

include_directories(${JNI_LIBS_DIR}/includes)

include_directories(${JNI_LIBS_DIR}/includes/soundtouch)

target_link_libraries(native-lib
                      OpenSLES
                      avutil swresample swscale avcodec avformat avfilter avdevice
                      ${log-lib} ${android-lib} )