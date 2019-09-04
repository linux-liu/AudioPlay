//
// Created by liuxin on 19-8-25.
//

#ifndef AUDIOPLAY_PLAYSTATUS_H
#define AUDIOPLAY_PLAYSTATUS_H

#define STATUS_PLAYING 1
#define STATUS_PAUSE 2
#define STAUTS_INIT 0
#define STAUTS_STOP 3

#define ERROR_IO 1001
#define ERROR_IO_INFO 1002
#define ERROR_FIND_STREAM_FAIL 1003
#define ERROR_CONTEXT_MALLOC_FAIL 1004
#define ERROR_COPY_PARM_FAIL 1005
#define ERROR_FIND_DECODE_FAIL 1006
#define ERROR_OPEN_DECODE_FAIL 1007

class PlayStatus {
public:
    int playStatus;
    bool isExit;
    bool isSeek;
public:
    PlayStatus();
    ~PlayStatus();


};


#endif //AUDIOPLAY_PLAYSTATUS_H
