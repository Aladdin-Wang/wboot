#include "check_ymodem_send_agent.h"
#include "perf_counter.h"
#include "wl_serve.h"
#include "usart.h"
#include <assert.h>
#include <string.h>

#undef this
#define this        (*ptThis)

__attribute__((aligned(32)))
static uint8_t s_chBuffer[1024] ;
static uint32_t count1 = 0, count2 = 0;
static bool ymodem_send_file_name(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t *phwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    count1++;
    count2 = 0;
    *phwSize = 128;
    memset(s_chBuffer, 0, sizeof(s_chBuffer));

    if(count1 > 2) {
        count1 = 0;
        return true;
    }

    sprintf((char *)pchBuffer, "%d_%s%c%d", count1, "123.txt", '\0', 102400);

    return true;
}

static bool ymodem_send_file_data(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t *phwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    count2++;
    *phwSize = 1024;

    if(count2 == (102400 / 1024 + 1)) {
        *phwSize = 102400 % 1024;
        memset(pchBuffer, count2 + 0X30, *phwSize);
        count2 = 0;
    } else {
        memset(pchBuffer, count2 + 0X30, 1024);
    }

    return true;
}

static uint16_t ymodem_read_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);
    return dequeue(&this.tByteInQueue, pchByte, hwSize);
}

bool ymodem_write_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_send_t *(ptThis) = (ymodem_send_t *)ptObj;
    assert(NULL != ptObj);

    uart_sent_data(&huart3, pchByte, hwSize);
    return true;
}


check_agent_ymodem_send_t *check_agent_ymodem_send_init(check_agent_ymodem_send_t *ptObj)
{
    check_agent_ymodem_send_t *(ptThis) = ptObj;
    assert(NULL != ptObj);

    this.tCheckAgent.pAgent = &this.tYmodemSend.parent;
    this.tCheckAgent.fnCheck = (check_hanlder_t *)ymodem_send;
    this.tCheckAgent.ptNext = NULL;

    this.tYmodemSend.pchQueueBuf = (char *)malloc(8);

    if(this.tYmodemSend.pchQueueBuf != NULL) {
        queue_init(&this.tYmodemSend.tByteInQueue, this.tYmodemSend.pchQueueBuf, 8);
    }

    static ymodem_ops_t s_tOps = {
        .pchBuffer = s_chBuffer,
        .hwSize = sizeof(s_chBuffer),
        .fnOnFileData = ymodem_send_file_data,
        .fnOnFilePath = ymodem_send_file_name,
        .fnReadData = ymodem_read_data,
        .fnWriteData = ymodem_write_data,
    };
    ymodem_init(&this.tYmodemSend.parent, &s_tOps);

    return ptObj;
}

