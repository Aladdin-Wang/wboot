#include "ymodem_send.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#undef this
#define this        (*ptThis)

__attribute__((aligned(32)))
static uint8_t s_chBuffer[1024] ;
static uint32_t count1 = 0, count2 = 0;
static uint16_t ymodem_send_file_name(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    count1++;
    count2 = 0;
    memset(s_chBuffer, 0, sizeof(s_chBuffer));

    if(count1 > 2) {
        count1 = 0;
        return 0;
    }

    sprintf((char *)pchBuffer, "%d_%s%c%d", count1, "123.txt", '\0', 102400);

    return hwSize;
}

static uint16_t ymodem_send_file_data(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    count2++;

    if(count2 == (102400 / 1024 + 1)) {
        hwSize = 102400 % 1024;
        memset(pchBuffer, count2 + 0X30, hwSize);
        count2 = 0;
    } else {
        memset(pchBuffer, count2 + 0X30, 1024);
    }

    return hwSize;
}

static uint16_t ymodem_read_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);
    return 0;
}

static uint16_t ymodem_write_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    return 0;
}


ymodem_send_t *ymodem_send_init(ymodem_send_t *ptObj)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    static ymodem_ops_t s_tOps = {
        .pchBuffer = s_chBuffer,
        .hwSize = sizeof(s_chBuffer),
        .fnOnFileData = ymodem_send_file_data,
        .fnOnFilePath = ymodem_send_file_name,
        .fnReadData = ymodem_read_data,
        .fnWriteData = ymodem_write_data,
    };
    ymodem_init(&this.parent, &s_tOps);

    return ptObj;
}

