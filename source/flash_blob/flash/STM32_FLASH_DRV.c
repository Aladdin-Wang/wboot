#include "../wl_flash_blob.h"
#include "STM32_FLASH_DEV.c"
/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int32_t Init(uint32_t adr, uint32_t clk, uint32_t fnc)
{
    HAL_FLASH_Unlock();
    return (0);
}

/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

static int32_t UnInit(uint32_t fnc)
{
    HAL_FLASH_Lock();
    return (0);
}

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

static int32_t EraseChip(void)
{
    int32_t result = 0;
    uint32_t PAGEError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_MASSERASE;
    #if defined(FLASH_BANK2_END)
    EraseInitStruct.Banks       = FLASH_BANK_2;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = 1;
    }

    #endif
    EraseInitStruct.Banks       = FLASH_BANK_1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = 1;
    }

    return result;
}

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
static int32_t EraseSector(uint32_t adr)
{
    int32_t result = 0;
    uint32_t PAGEError = 0;

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;
    #if defined(FLASH_BANK2_END)
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = adr;
    EraseInitStruct.NbPages     = 1;

    if(adr > FLASH_BANK1_END) {
        EraseInitStruct.Banks       = FLASH_BANK_2;
    } else {
        EraseInitStruct.Banks       = FLASH_BANK_1;
    }

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = 1;
    }

    #else
    #if defined(STM32F303x8) || defined(STM32F105xC) || defined(STM32F103xE)
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = adr;
    EraseInitStruct.NbPages     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = 1;
    }

    #endif
    #if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx)|| \
    defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F413xx)

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    /* Get the 1st sector to erase */
    EraseInitStruct.Sector = get_flash_sector(adr);
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.NbSectors     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = 1;
    }

    #endif
    #if defined(STM32H743xx)
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = (adr - FLASH_BANK1_BASE) / FLASH_SECTOR_SIZE;;
    EraseInitStruct.NbSectors   = 1;
    EraseInitStruct.Banks       = FLASH_BANK_1;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        result = 1;
    }

    #endif
    #endif
    return result;
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
static int32_t ProgramPage(uint32_t addr, uint32_t sz, uint8_t* buf)
{
    int32_t result = 0;

    #if defined(STM32F303x8) || defined(STM32F105xC) || defined(STM32F103xE)
    uint32_t end_addr   = addr + sz;

    while (addr < end_addr) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *((uint32_t *)buf)) == HAL_OK) {
            if (*(uint32_t *)addr != *(uint32_t *)buf) {
                result = 1;
                break;
            }

            addr += 4;
            buf  += 4;
        } else {
            result = 1;
            break;
        }
    }

    #endif
    #if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx)|| \
    defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F413xx)
    uint32_t written_size = 0;
    uint32_t write_size = 0;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    while (written_size < sz) {
        if (((addr + written_size) % 4 == 0) && (sz - written_size >= 4)) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + written_size, *((uint32_t *)(buf + written_size))) == HAL_OK) {
                if (*(uint32_t *)(addr + written_size) != *(uint32_t *)(buf + written_size)) {
                    result = 1;
                    break;
                }
            } else {
                result = 1;
                break;
            }

            write_size = 4;
        } else if (((addr + written_size) % 2 == 0) && (sz - written_size >= 2)) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + written_size, *((uint16_t *)(buf + written_size))) == HAL_OK) {
                if (*(uint16_t *)(addr + written_size) != *(uint16_t *)(buf + written_size)) {
                    result = 1;
                    break;
                }
            } else {
                result = 1;
                break;
            }

            write_size = 2;
        } else {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + written_size, *((uint8_t *)(buf + written_size))) == HAL_OK) {
                if (*(uint8_t *)(addr + written_size) != *(uint8_t *)(buf + written_size)) {
                    result = 1;
                    break;
                }
            } else {
                result = 1;
                break;
            }

            write_size = 1;
        }

        written_size += write_size;
    }

    #endif
    #if defined(STM32H743xx)
    uint32_t write_granularity = FLASH_NB_32BITWORD_IN_FLASHWORD * 4;
    uint32_t write_size = write_granularity;
    uint32_t end_addr   = addr + sz - 1, write_addr;
    uint8_t write_buffer[32] = {0};
    write_addr = (uint32_t)buf;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR);

    while (addr < end_addr) {
        if(end_addr - addr + 1 < write_granularity) {
            write_size = end_addr - addr + 1;

            for(size_t i = 0; i < write_size; i++) {
                write_buffer[i] = *((uint8_t *)(write_addr + i));
            }

            write_addr = (uint32_t)((uint32_t *)write_buffer);
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr, write_addr) == HAL_OK) {
            for(uint8_t i = 0; i < write_size; i++) {
                if (*(uint8_t *)(addr + i) != *(uint8_t *)(write_addr + i)) {
                    result = 1;
                    break;
                }
            }

            addr += write_granularity;
            write_addr  += write_granularity;
        } else {
            result = 1;
            break;
        }
    }

    #endif
    return result;
}

const  flash_blob_t  onchip_flash_device = {
    .tFlashops.Init = Init,
    .tFlashops.UnInit = UnInit,
    .tFlashops.EraseChip = EraseChip,
    .tFlashops.EraseSector = EraseSector,
    .tFlashops.Program = ProgramPage,
    .tFlashops.Read = NULL,
    .ptFlashDev = &FlashDevice,
};

