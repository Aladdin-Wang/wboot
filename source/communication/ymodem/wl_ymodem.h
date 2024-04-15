/****************************************************************************
*  Copyright 2022 kk (https://github.com/Aladdin-Wang)                                    *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

#ifndef __SERVE_YMODEM_H_
#define __SERVE_YMODEM_H_

/* Inclusion of necessary system headers and configuration headers */
#include "./app_cfg.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* C++ compatibility for C headers */
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    bool bAutoReset;
    bool bIsSet;
}ymodem_event_t;

typedef enum {
    PACKET_CAN                      = 3,
    PACKET_EOT                      = 2,
    PACKET_ON_GOING                 = 1,     //!< on-going
    PACKET_CPL                      = 0,     //!< complete
    PACKET_TIMEOUT                  = -1,    //!< read timeout
    PACKET_INCORRECT_HEADER         = -2,    //!< incorrect head char
    PACKET_INCORRECT_PACKET_NUMBER  = -3,    //!< incorrect packet number
    PACKET_DUPLICATE_PACKET_NUMBER  = -4,    //!< duplicate packet number
    PACKET_INCORRECT_CHECKOUT       = -5,    //!< incorrect checkout
    PACKET_INCORRECT_NBlk           = -6,    //!< incorrect NBlk
    PACKET_FAIL                     = -7,    //!< packet faile
} ymodem_state_t;

/* Struct for holding YMODEM package read data  */
typedef struct ymodem_read_timeout_t {
    uint8_t  chState;
    uint16_t hwRemainSize;	
	  uint16_t hwIndex;
	  int64_t lTimeCountms;
} ymodem_read_timeout_t;

/* Struct for holding YMODEM package state information */
typedef struct ymodem_package_t {
    uint8_t chState; /* Current state of the package reception/transmission */
    uint8_t chByte; /* Byte value used for various operations */
    uint8_t chHead; /* Head byte for the packet */
    uint8_t chBlk; /* Block number */
    uint8_t chNBlk; /* Complement of the block number */
    uint8_t chCheck[2]; /* Checksum/CRC bytes */
    uint16_t hwCheck; /* Calculated checksum/CRC value */
} ymodem_package_t;

/*Pre Declaration*/
typedef struct ymodem_ops_t ymodem_ops_t;
/* Struct for handling the YMODEM protocol interaction */
typedef struct ymodem_t {
    uint8_t chState; /* Current state of the YMODEM operation */
    uint8_t chByte; /* Byte value used for control operations */
    uint8_t chTryCount; /* Counter for retry attempts */
    uint8_t chPacketNum; /* Packet number in sequence */
    ymodem_ops_t *ptOps; /* virtual function for YMODEM operations */
    ymodem_package_t tPackage; /* Package related information */
	  ymodem_read_timeout_t tReadDataTimeout;
	  ymodem_event_t tEvent;
} ymodem_t;

/* Callback type definitions for various YMODEM operations */
typedef bool (ymodem_call_back)(ymodem_t *ptObj,uint8_t *pchBuffer, uint16_t *phwSize);
typedef uint16_t (ymodem_read_call_back)(ymodem_t *ptObj,uint8_t* pchByte, uint16_t hwSize);
typedef bool (ymodem_write_call_back)(ymodem_t *ptObj,uint8_t* pchByte, uint16_t hwSize);

/* virtual function table for encapsulating YMODEM operation functions */
typedef struct ymodem_ops_t {
    uint16_t hwSize; /* Size of the data block */
    uint8_t *pchBuffer; /* Pointer to data buffer */
    ymodem_call_back *fnOnFilePath; /* Callback for file path operation */
    ymodem_call_back *fnOnFileData; /* Callback for file data operation */
    ymodem_read_call_back *fnReadData; /* Callback for reading data */
    ymodem_write_call_back *fnWriteData; /* Callback for writing data */
} ymodem_ops_t;


static inline uint16_t __ymodem_read_data_timeout(ymodem_t *ptObj,uint8_t* pchByte, uint16_t hwSize)
{
	return (*ptObj->ptOps->fnReadData)(ptObj,pchByte,hwSize);
}

static inline bool __ymodem_write_data(ymodem_t *ptObj,uint8_t* pchByte, uint16_t hwSize)
{
	return (*ptObj->ptOps->fnWriteData)(ptObj,pchByte,hwSize);
}

static inline bool __file_path(ymodem_t *ptObj,uint8_t *pchBuffer, uint16_t *phwSize)
{
	return (*ptObj->ptOps->fnOnFilePath)(ptObj,pchBuffer,phwSize);	
}

static inline bool __file_data(ymodem_t *ptObj,uint8_t *pchBuffer, uint16_t *phwSize)
{
	return (*ptObj->ptOps->fnOnFileData)(ptObj,pchBuffer,phwSize);	
}

static inline uint16_t __get_size(ymodem_t *ptObj)
{
	return ptObj->ptOps->hwSize;		
}

static inline uint16_t * __get_size_addr(ymodem_t *ptObj)
{
	return &ptObj->ptOps->hwSize;		
}

static inline void __set_size(ymodem_t *ptObj,uint16_t hwSize)
{
	ptObj->ptOps->hwSize = hwSize;		
}

static inline uint8_t * __get_buffer_addr(ymodem_t *ptObj)
{
	return ptObj->ptOps->pchBuffer;		
}

/* External function declarations for CRC calculation, YMODEM receive, and send operations */
extern uint16_t ymodem_crc16(unsigned char *q, int len);
extern ymodem_state_t ymodem_receive(ymodem_t *ptThis);
extern ymodem_state_t ymodem_send(ymodem_t *ptThis);

/* Initialization function for the YMODEM protocol */
extern bool ymodem_init(ymodem_t *ptThis,ymodem_ops_t *ptOps);

/* End of C++ compatibility block */
#ifdef __cplusplus
}
#endif

/* End of include guard */
#endif