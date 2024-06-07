#ifndef APPLICATIONS_CHECK_AGENT_YMODEM_RECEIVE_H_
#define APPLICATIONS_CHECK_AGENT_YMODEM_RECEIVE_H_
#include "wl_ymodem.h"
#include "../shell/signals_slots/wl_signals_slots.h"
#include "../../check_agent_engine/wl_check_agent_engine.h"
#include "../../bootloader/bootloader.h"

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#endif
		
typedef struct ymodem_receive_t {
		SIG_SLOT_OBJ;
    ymodem_t      parent;
    char         *pchQueueBuf;
    byte_queue_t  tByteInQueue;
    char         *pchFileName;
    char         *pchFileSize;
    uint32_t      wFileSize;
    uint32_t      wOffSet;
} ymodem_receive_t;

typedef struct check_agent_ymodem_recive_t {
    check_agent_t tCheckAgent;
    ymodem_receive_t tYmodemReceive;
} check_agent_ymodem_recive_t;

signals(ymodem_rec_sig,ymodem_receive_t *ptThis,
      args(              
            uint8_t *pchByte,
            uint16_t hwLen
          ));

extern check_agent_ymodem_recive_t *check_agent_ymodem_receive_init(check_agent_ymodem_recive_t *ptObj,uint8_t *pchBuffer,uint16_t hwSize);

#endif /* APPLICATIONS_CHECK_AGENT_XMODEM_H_ */
