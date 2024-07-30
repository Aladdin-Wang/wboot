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
#include <stdio.h>

/* C++ compatibility for C headers */
#ifdef __cplusplus
extern "C" {
#endif
#define PRINT_ERROR(message) printf("\nError: %s in file %s at line %d\n", message, __FILE__, __LINE__)

typedef enum {
    PACKET_CPL                      = 0,     //!< complete
    PACKET_ON_GOING                 = 1,     //!< on-going
    PACKET_EOT                      = 2,
    PACKET_CAN                      = 3,
    PACKET_TIMEOUT                  = 4,     //!< read timeout
    PACKET_INCORRECT_HEADER         = 5,     //!< incorrect head char
    PACKET_INCORRECT_PACKET_NUMBER  = 6,     //!< incorrect packet number
    PACKET_DUPLICATE_PACKET_NUMBER  = 7,     //!< duplicate packet number
    PACKET_INCORRECT_CHECKOUT       = 8,     //!< incorrect checkout
    PACKET_INCORRECT_NBlk           = 9,     //!< incorrect NBlk
    PACKET_INCORRECT_SIZE           = 10,    //!< incorrect SIZE
    PACKET_FAIL                     = 11
} ymodem_state_t;

typedef struct ymodem_t ymodem_t;
/* Callback type definitions for various YMODEM operations */
typedef uint16_t (ymodem_call_back)(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize);

/* virtual function table for encapsulating YMODEM operation functions */
typedef struct ymodem_ops_t {
    uint16_t                   hwSize; /* Size of the data block */
    uint8_t                    *pchBuffer; /* Pointer to data buffer */
    ymodem_call_back           *fnOnFilePath; /* Callback for file path operation */
    ymodem_call_back           *fnOnFileData; /* Callback for file data operation */
    ymodem_call_back           *fnReadData; /* Callback for reading data */
    ymodem_call_back           *fnWriteData; /* Callback for writing data */
} ymodem_ops_t;

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
	  uint16_t hwWriteLen;
} ymodem_package_t;

/* Struct for handling the YMODEM protocol interaction */
typedef struct ymodem_t {
    uint8_t chState; /* Current state of the YMODEM operation */
    uint8_t chByte; /* Byte value used for control operations */
    uint8_t chTryCount; /* Counter for retry attempts */
    uint8_t chPacketNum; /* Packet number in sequence */
    ymodem_ops_t tOps; /* virtual function for YMODEM operations */
    ymodem_package_t tPackage; /* Package related information */
    ymodem_read_timeout_t tReadDataTimeout;
} ymodem_t;

static inline uint16_t __ymodem_read_data_timeout(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    return (*ptObj->tOps.fnReadData)(ptObj, pchByte, hwSize);
}

static inline uint16_t __ymodem_write_data(ymodem_t *ptObj, uint8_t* pchByte, uint16_t hwSize)
{
    return (*ptObj->tOps.fnWriteData)(ptObj, pchByte, hwSize);
}

static inline uint16_t __file_path(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize)
{
    return (*ptObj->tOps.fnOnFilePath)(ptObj, pchBuffer, hwSize);
}

static inline uint16_t __file_data(ymodem_t *ptObj, uint8_t *pchBuffer, uint16_t hwSize)
{
    return (*ptObj->tOps.fnOnFileData)(ptObj, pchBuffer, hwSize);
}

static inline uint16_t __get_size(ymodem_t *ptObj)
{
    return ptObj->tOps.hwSize;
}

static inline void __set_size(ymodem_t *ptObj, uint16_t hwSize)
{
    ptObj->tOps.hwSize = hwSize;
}

static inline uint8_t * __get_buffer_addr(ymodem_t *ptObj)
{
    return ptObj->tOps.pchBuffer;
}

typedef enum {
    fsm_ym_err              = -1,    //!< fsm error, error code can be get from other interface
    fsm_ym_cpl              = 0,     //!< fsm complete
    fsm_ym_on_going         = 1,     //!< fsm on-going
    fsm_ym_wait_for_obj     = 2,     //!< fsm wait for object
    fsm_ym_asyn             = 3,     //!< fsm asynchronose complete, you can check it later.
    fsm_ym_req_drop         = 4,
    fsm_ym_req_timeout      = 5,
} fsm_ym_t;

/* External function declarations for CRC calculation, YMODEM receive, and send operations */
extern uint16_t ymodem_crc16(unsigned char *q, int len);
extern fsm_ym_t ymodem_receive(ymodem_t *ptThis);
extern fsm_ym_t ymodem_send(ymodem_t *ptThis);

/* Initialization function for the YMODEM protocol */
extern void ymodem_init(ymodem_t *ptThis, ymodem_ops_t *ptOps);

/* End of C++ compatibility block */
#ifdef __cplusplus
}
#endif

/* End of include guard */
#endif