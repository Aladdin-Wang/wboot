#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_
#include ".\app_cfg.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char chProjectName[16];
    char chHardWareVersion[16];
    char chSoftBootVersion[16];
    char chSoftAppVersion[16];
    char wBoardId;
    char chPortName[16];
    int wPortBaudrate;
    int wRestartTime;
} msgSig_t;
typedef struct {
    union {
        msgSig_t sig;
        char B[sizeof(msgSig_t)];
    } msg_data;
} user_data_t;

#endif






