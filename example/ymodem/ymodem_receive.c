#include "ymodem_receive.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#undef this
#define this        (*ptThis)

__attribute__((aligned(32)))
uint8_t s_chQueueBuffer[1024] ;

static uint16_t ymodem_recv_file_name(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize)
{
    ymodem_receive_t *(ptThis) = (ymodem_receive_t *)ptObj;
    assert(NULL != ptObj);
	
    this.wOffSet = 0;
    strcpy(this.chFileName, (char *)&pchBuffer[0]);
    this.pchFileSize = (char *)&pchBuffer[strlen(this.chFileName) + 1];
    this.wFileSize = atol(this.pchFileSize);

    printf("Ymodem file_name:%s \r\n", this.chFileName);
    printf("Ymodem file_size:%d \r\n", this.wFileSize);

    return hwSize;
}

static uint16_t ymodem_recv_file_data(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize)
{
    ymodem_receive_t *(ptThis) = (ymodem_receive_t *)ptObj;
    assert(NULL != ptObj);

    uint32_t  wRemainLen = this.wFileSize - this.wOffSet;
    uint32_t  wWriteLen = hwSize;

    if(wWriteLen > wRemainLen) {
        wWriteLen = wRemainLen;
    }

    return hwSize;
}

static uint16_t ymodem_read_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_receive_t *(ptThis) = (ymodem_receive_t *)ptObj;
    assert(NULL != ptObj);
    return 0;
}

static uint16_t ymodem_write_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_receive_t *(ptThis) = (ymodem_receive_t *)ptObj;
    assert(NULL != ptObj);

    return 0;
}

ymodem_receive_t *ymodem_receive_init(ymodem_receive_t *ptObj)
{
	ymodem_receive_t *(ptThis) = (ymodem_receive_t *)ptObj;
    assert(NULL != ptObj);
	
    ymodem_ops_t s_tOps = {
        .pchBuffer = s_chQueueBuffer,
        .hwSize = sizeof(s_chQueueBuffer),
        .fnOnFileData = ymodem_recv_file_data,
        .fnOnFilePath = ymodem_recv_file_name,
        .fnReadData = ymodem_read_data,
        .fnWriteData = ymodem_write_data
    };
    ymodem_init(&this.parent, &s_tOps);
    return ptObj;
}

