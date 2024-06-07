#include "wl_ymodem_ota.h"

#include <assert.h>

#undef this
#define this        (*ptThis)
	
__attribute__((aligned(32)))
char s_chQueueBuffer[2048] ;

static bool ymodem_recv_file_name(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t *phwSize)
{
    ymodem_receive_t *(ptThis) = container_of(ptObj, ymodem_receive_t, parent);
    assert(NULL != ptObj);

    this.wOffSet = 0;
    this.pchFileName = (char *)&pchBuffer[0];
    this.pchFileSize = (char *)&pchBuffer[strlen(this.pchFileName) + 1];
    this.wFileSize = atol(this.pchFileSize);

    if(this.wFileSize <= 0) {
        *phwSize = 0;
        return true;
    }

    printf("Ymodem file_name:%s \r\n", this.pchFileName);
    printf("Ymodem file_size:%d \r\n", this.wFileSize);


    if(strlen(tUserData.msg_data.sig.chProjectName) > 0) {
        if(strcmp(tUserData.msg_data.sig.chProjectName, this.pchFileName) != 0 ) {
            printf("Firmware Name Check failure.");
            return false;
        }
    }

    if( APP_PART_SIZE  < this.wFileSize) {
        printf("file size outrange flash size. \r\n");
        return false;
    }

    if(target_flash_init(APP_PART_ADDR) == false) {
        printf("target flash uninit. \r\n");
        return false;
    }

    uint32_t wEraseSize = target_flash_erase(APP_PART_ADDR, this.wFileSize);

    if( wEraseSize < this.wFileSize) {
        printf("target flash erase error. \r\n");
        return false;
    }
    target_flash_uninit(APP_PART_ADDR);
    printf("flash erase success:%d \r\n", wEraseSize);

    return true;
}

static bool ymodem_recv_file_data(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t *phwSize)
{
    ymodem_receive_t *(ptThis) = container_of(ptObj, ymodem_receive_t, parent);
    assert(NULL != ptObj);

    uint32_t  wRemainLen = this.wFileSize - this.wOffSet;
    uint32_t  wWriteLen = *phwSize;

    if(wWriteLen > wRemainLen) {
        wWriteLen = wRemainLen;
    }
    target_flash_init(APP_PART_ADDR);
    uint16_t wWriteSize = target_flash_write(APP_PART_ADDR + this.wOffSet, pchBuffer, wWriteLen);
    target_flash_uninit(APP_PART_ADDR);
    if( wWriteSize != wWriteLen) {
        printf("target flash write data error. 0x%x.\r\n", wWriteSize);
        return false;
    }

    uint16_t hwWriteCheck = ymodem_crc16(pchBuffer, wWriteLen);
    uint16_t wReadSize = target_flash_read(APP_PART_ADDR + this.wOffSet, pchBuffer, wWriteLen);

    if( wReadSize != wWriteLen) {
        printf("target flash wReadSize data error. 0x%x.\r\n", wReadSize);
        return false;
    }

    uint16_t hwReadCheck = ymodem_crc16(pchBuffer, wWriteLen);

    if(hwWriteCheck != hwReadCheck) {
        printf("Check error. WriteCheck:0x%x ReadCheck:0x%x.\r\n", hwWriteCheck, hwReadCheck);
        return false;
    }

    this.wOffSet += wWriteLen;

    if(this.wOffSet == this.wFileSize) {
        printf("Download firmware to flash success.\n");	
    }

    return true;
}

static uint16_t ymodem_read_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_receive_t *(ptThis) = container_of(ptObj, ymodem_receive_t, parent);
    assert(NULL != ptObj);
    return dequeue(&this.tByteInQueue, pchByte, hwSize);
}

static bool ymodem_write_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    ymodem_receive_t *(ptThis) = container_of(ptObj, ymodem_receive_t, parent);
    assert(NULL != ptObj);
	  emit(ymodem_rec_sig, ptThis,
		    args(
			     pchByte,
			     hwSize
		 ));

    return true;
}



check_agent_ymodem_recive_t *check_agent_ymodem_receive_init(check_agent_ymodem_recive_t *ptObj,uint8_t *pchBuffer,uint16_t hwSize)
{
    check_agent_ymodem_recive_t *(ptThis) = ptObj;
    assert(NULL != ptObj);
    this.tCheckAgent.pAgent = &this.tYmodemReceive.parent;
    this.tCheckAgent.fnCheck = (check_hanlder_t *)ymodem_receive;
    this.tCheckAgent.ptNext = NULL;

    this.tYmodemReceive.pchQueueBuf = s_chQueueBuffer;

    if(this.tYmodemReceive.pchQueueBuf != NULL) {
        queue_init(&this.tYmodemReceive.tByteInQueue, this.tYmodemReceive.pchQueueBuf, sizeof(s_chQueueBuffer));
    }

    ymodem_ops_t s_tOps = {
        .pchBuffer = pchBuffer,
        .hwSize = hwSize,
        .fnOnFileData = ymodem_recv_file_data,
        .fnOnFilePath = ymodem_recv_file_name,
        .fnReadData = ymodem_read_data,
        .fnWriteData = ymodem_write_data,
    };
    ymodem_init(&this.tYmodemReceive.parent, &s_tOps);

    return ptObj;
}

