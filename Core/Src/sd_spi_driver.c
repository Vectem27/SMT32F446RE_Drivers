/*
 * sd_spi_driver.h
 *
 *  Created on: Sep 27, 2025
 *      Author: Vectem
 */

#include "sd_spi_driver.h"

#include <stdio.h>

/**
 * @brief  Data response sent for CMD24
 */
typedef enum _SD_DataResponse
{
    SD_RESPONSE_MASK = 0x0E, /*!< any response value bits have to be masked by this */
    SD_RESPONSE_ACCEPTED = 0x04, /*!< data accepted */
    SD_RESPONSE_REJECTED_CRC = 0x0A, /*!< data rejected due to CRC error */
    SD_RESPONSE_REJECTED_ERR = 0x0C /*!< data rejected due to write error */
} SD_DataResponse;

/**
 * @brief  Data response error
 */
typedef enum _SD_DataError
{
    SD_DATA_TOKEN_OK = 0x00,
    SD_DATA_TOKEN_ERROR = 0x01,
    SD_DATA_TOKEN_CC_ERROR = 0x02,
    SD_DATA_TOKEN_ECC_FAILURE = 0x04,
    SD_DATA_TOKEN_OUT_OF_RANGE = 0x08,
    SD_DATA_TOKEN_CARD_LOCKED = 0x10,
} SD_DataError;

/**
 * @brief  Commands: CMDxx = CMD-number | 0x40
 */
/*
 class 0 (basic):
 CMD0        CMD2        CMD3        CMD4
 CMD7        CMD8        CMD9        CMD10
 CMD11       CMD12       CMD13       CMD15
 class 2 (block read):
 CMD16       CMD17       CMD18       CMD19       CMD20       CMD23
 class 4 (block write):
 CMD16       CMD20       CMD23       CMD24       CMD25       CMD27
 class 5 (erase):
 CMD32       CMD33       CMD38
 class 6 (write protection):
 CMD28       CMD29       CMD30
 class 7 (lock card):
 CMD16       CMD40       CMD42
 class 8 (application-specific):
 CMD55       CMD56
 ACMD6       ACMD13  ACMD22  ACMD23  ACMD41  ACMD42  ACMD51
 class 9 (I/O mode):
 CMD5        CMD52       CMD53
 class 10 (switch):
 CMD6        CMD34       CMD35       CMD36       CMD37       CMD50       CMD57
 all other classes are reserved
 */
typedef enum _SD_CMD
{
    SD_CMD_GO_IDLE_STATE = 0,     // CMD0  = 0x40, ARG=0x00000000, CRC=0x95
    SD_CMD_SEND_OP_COND = 1,          // CMD1  = 0x41
    SD_CMD_SEND_IF_COND = 8,          // CMD8  = 0x48, ARG=0x000001AA, CRC=0x87
    SD_CMD_SEND_CSD = 9,          // CMD9  = 0x49
    SD_CMD_SEND_CID = 10,     // CMD10 = 0x4A
    SD_CMD_STOP_TRANSMISSION = 12,     // CMD12 = 0x4C
    SD_CMD_SET_BLOCKLEN = 16,     // CMD16 = 0x50
    SD_CMD_READ_SINGLE_BLOCK = 17,     // CMD17 = 0x51
    SD_CMD_READ_MULT_BLOCK = 18,     // CMD18 = 0x52
    SD_CMD_SET_BLOCK_COUNT = 23,     // CMD23 = 0x57
    SD_CMD_WRITE_SINGLE_BLOCK = 24,     // CMD24 = 0x58
    SD_CMD_WRITE_MULT_BLOCK = 25,     // CMD25 = 0x59
    SD_CMD_ERASE_BLOCK_START = 32,     // CMD32 = 0x60
    SD_CMD_ERASE_BLOCK_END = 33,     // CMD33 = 0x61
    SD_CMD_ERASE = 38,     // CMD38 = 0x66
    SD_CMD_SEND_APP = 55,     // CMD55 = 0x77, ARG=0x00000000, CRC=0x65
    SD_CMD_READ_OCR = 58,     // CMD58 = 0x7A, ARG=0x00000000, CRC=0xFF
} SD_CMD;

typedef enum _SD_ACMD
{
    SD_ACMD_STATUS = 13,     // ACMD13= 0x4D
    SD_ACMD_ACTIVATE_INIT = 41,     // ACMD41= 0x69, ARG=0x40000000, CRC=0x77
    SD_ACMD_SEND_SCR = 51,     // ACMD51= 0x73
} SD_ACMD;

/**
 * @brief  Dummy byte
 */
#define SD_DUMMY_BYTE       0xFF

/**
 * @brief  Number of 8-bit cycles for RUMP UP phase
 */
#define SD_NUM_TRIES_RUMPUP ((uint32_t)2500)

/**
 * @brief  Maximum number of tries to send a command
 */
#define SD_NUM_TRIES        ((uint16_t)300)

/**
 * @brief  Maximum number of tries until ACMD41/CMD1 initializes SD card
 * It means a time until "In Idle State" flag is set during initialization
 * For information:
 * ~ 11000 for Kingston 4Gb
 * ~ 10000 for SanDisk  1Gb
 * ~  6000 for Samsung  8Gb
 */
#define SD_NUM_TRIES_INIT   ((uint16_t)20000)

/**
 * @brief  Maximum number of tries to receive data transmission token
 * It means a time before data transmission starts
 * For information:
 * ~  300 for SanDisk  1Gb
 * ~  600 for Kingston 4Gb
 * ~  900 for SP       4Gb
 * ~  500 for Samsung  8Gb
 * ~  300 for Lexar    4Gb
 */
#define SD_NUM_TRIES_READ   ((uint16_t)2000)

/**
 * @brief  Maximum number of tries until SD card writes data
 * It means a time while BUSY flag is set, i.e. card writes the data received
 * For information:
 * ~  6100 for Kingston 4Gb
 * ~  4600 for Lexar    4Gb
 * ~ 80000 for SP       4Gb (9000)
 * ~ 10000 for SanDisk  1Gb
 * ~119000 for Samsung  8Gb
 */
#define SD_NUM_TRIES_WRITE  ((uint32_t)1000000)

/**
 * @brief  Maximum number of tries until SD card erases data
 * It means a time while BUSY flag is set, i.e. card erases sectors
 * For information:
 * ~  6100 for Kingston 4Gb
 * ~  5200 for Lexar    4Gb
 * ~ 10300 for SP       4Gb
 * ~   N/A for SanDisk  1Gb
 * ~120000 for Samsung  8Gb
 */
#define SD_NUM_TRIES_ERASE  ((uint32_t)1000000)

/**
 * @brief  Start Data tokens:
 *         Tokens (necessary because at nop/idle (and CS active) only 0xff is
 *         on the data/command line)
 */
#define SD_DATA_BLOCK_READ_START           0xFE  /*!< Data token start byte, Start Single/Multiple Block Read */
#define SD_DATA_SINGLE_BLOCK_WRITE_START   0xFE  /*!< Data token start byte, Start Single Block Write */
#define SD_DATA_MULTIPLE_BLOCK_WRITE_START 0xFC  /*!< Data token start byte, Start Multiple Block Write */
#define SD_DATA_MULTIPLE_BLOCK_WRITE_STOP  0xFD  /*!< Data token stop byte, Stop Multiple Block Write */

/**
 * @brief  Write a byte on the SD.
 * @param  Data: byte to send.
 * @retval None
 */
void SD_WriteByte(SD_SPI_Handle *sd, uint8_t data)
{
    HAL_SPI_Transmit(sd->init.hspi, &data, 1, HAL_MAX_DELAY);
}

/**
 * @brief  Read a byte from the SD.
 * @param  None
 * @retval The received byte.
 */
uint8_t SD_ReadByte(SD_SPI_Handle *sd)
{
    uint8_t dummy = SD_DUMMY_BYTE;
    uint8_t data;
    HAL_SPI_TransmitReceive(sd->init.hspi, &dummy, &data, 1, HAL_MAX_DELAY);
    return data;
}

void SD_Bus_Hold(SD_SPI_Handle *sd)
{ /* Select SD Card: set SD chip select pin low */
    HAL_GPIO_WritePin(sd->init.CS_Port, sd->init.CS_Pin, GPIO_PIN_RESET);
}

void SD_Bus_Release(SD_SPI_Handle *sd)
{ /* Deselect SD Card: set SD chip select pin high */
    HAL_GPIO_WritePin(sd->init.CS_Port, sd->init.CS_Pin, GPIO_PIN_SET);
    SD_ReadByte(sd); /* send dummy byte: 8 Clock pulses of delay */
}

/**
 * @brief  Send a command to SD card and receive R1 response
 * @param  Cmd: Command to send to SD card
 * @param  Arg: Command argument
 * @param  Crc: CRC
 * @retval R1 response byte
 */
static SD_Error SD_SendCmd(SD_SPI_Handle *sd, uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t res;
    uint16_t i = SD_NUM_TRIES;
    /* send a command */
    SD_WriteByte(sd, (cmd & 0x3F) | 0x40); /*!< byte 1 */
    SD_WriteByte(sd, (uint8_t) (arg >> 24)); /*!< byte 2 */
    SD_WriteByte(sd, (uint8_t) (arg >> 16)); /*!< byte 3 */
    SD_WriteByte(sd, (uint8_t) (arg >> 8)); /*!< byte 4 */
    SD_WriteByte(sd, (uint8_t) arg); /*!< byte 5 */
    SD_WriteByte(sd, crc | 0x01); /*!< byte 6: CRC */

    /* a byte received immediately after CMD12 should be discarded... */
    if (cmd == SD_CMD_STOP_TRANSMISSION)
        SD_ReadByte(sd);
    /* SD Card responds within Ncr (response time),
     which is 0-8 bytes for SDSC cards, 1-8 bytes for MMC cards */
    do
    {
        res = SD_ReadByte(sd);
        /* R1 response always starts with 7th bit set to 0 */
    } while ((res & SD_CHECK_BIT) != 0x00 && i-- > 0);
    return (SD_Error) res;
}

/**
 * @brief  Get 4 bytes of R3 or R7 response
 * @param  pres: Pointer to uint32_t variable for result
 * @retval None
 */
static void SD_GetResponse4b(SD_SPI_Handle *sd, uint8_t *pres)
{
    pres[3] = SD_ReadByte(sd);
    pres[2] = SD_ReadByte(sd);
    pres[1] = SD_ReadByte(sd);
    pres[0] = SD_ReadByte(sd);
}

/**
 * @brief  Some commands take longer time and respond with R1b response,
 *         so we have to wait until 0xFF recieved (MISO is set to HIGH)
 * @retval Nonzero if required state wasn't recieved
 */
static SD_Error SD_WaitReady(SD_SPI_Handle *sd)
{
    uint16_t i = SD_NUM_TRIES;
    while (i-- > 0)
    {
        if (SD_ReadByte(sd) == 0xFF)
            return SD_RESPONSE_NO_ERROR;
    }
    return SD_RESPONSE_FAILURE;
}

/**
 * @brief  Wait until data transmission token is received
 * @retval Data transmission token or 0xFF if timeout occured
 */
static uint8_t SD_WaitBytesRead(SD_SPI_Handle *sd)
{
    uint16_t i = SD_NUM_TRIES_READ;
    uint8_t b;
    do
    {
        b = SD_ReadByte(sd);
    } while (b == 0xFF && i-- > 0);

    if (b != 0xFF)
        printf(" [[ READ delay %d ]] ", SD_NUM_TRIES_READ - i);
    else
        printf(" [[ READ delay was not enough ]] ");

    return b;
}

/**
 * @brief  Writing data into flash takes even longer time and it responds with R1b response,
 *         so we have to wait until 0xFF recieved (MISO is set to HIGH)
 * @retval Nonzero if required state wasn't recieved
 */
static SD_Error SD_WaitBytesWritten(SD_SPI_Handle *sd)
{
    uint32_t i = SD_NUM_TRIES_WRITE;
    while (i-- > 0)
    {
        if (SD_ReadByte(sd) == 0xFF)
        {
            printf(" [[ WRITE delay %lu ]] ", SD_NUM_TRIES_WRITE - i);
            return SD_RESPONSE_NO_ERROR;
        }
    }
    printf(" [[ WRITE delay was not enough ]] ");
    return SD_RESPONSE_FAILURE;
}

/**
 * @brief  Erasing data into flash takes some time and it responds with R1b response,
 *         so we have to wait until 0xFF recieved (MISO is set to HIGH)
 * @retval Nonzero if required state wasn't recieved
 */
static SD_Error SD_WaitBytesErased(SD_SPI_Handle *sd)
{
    uint32_t i = SD_NUM_TRIES_ERASE;
    while (i-- > 0)
    {
        if (SD_ReadByte(sd) == 0xFF)
        {
            printf(" [[ ERASE delay %lu ]] ", SD_NUM_TRIES_ERASE - i);
            return SD_RESPONSE_NO_ERROR;
        }
    }
    printf(" [[ ERASE delay was not enough ]] ");
    return SD_RESPONSE_FAILURE;
}

/**
 * @brief  Recieve data from SD Card
 * @param  data: Pre-allocated data buffer
 * @param  len: Number of bytes to receive
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
static SD_Error SD_ReceiveData(SD_SPI_Handle *sd, uint8_t *data, uint16_t len)
{
    uint16_t i = 0;
    uint8_t b;

    /* some cards need time before transmitting the data... */
    b = SD_WaitBytesRead(sd);

    if (b != SD_DATA_BLOCK_READ_START)
        return SD_RESPONSE_FAILURE;

    for (i = 0; i < len; ++i)
        data[i] = SD_ReadByte(sd);

    /* CRC Reading */
    SD_ReadByte(sd);
    SD_ReadByte(sd);

    return SD_RESPONSE_NO_ERROR;

}

SD_Error SD_GetCSDRegister(SD_SPI_Handle *sd, SD_CSD *SD_csd)
{
    SD_Error state;
    uint8_t CSD_Tab[16];

    state = SD_WaitReady(sd); /* make sure card is ready before we go further... */
    if (state != SD_RESPONSE_NO_ERROR)
        return SD_RESPONSE_FAILURE;

    /* request CSD register (send CMD9)... */
    state = SD_SendCmd(sd, SD_CMD_SEND_CSD, 0x00000000, 0xFF);
    if (state != SD_RESPONSE_NO_ERROR)
        return SD_RESPONSE_FAILURE;
    state = SD_ReceiveData(sd, CSD_Tab, 16); /* receive CSD register data */

    SD_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6; /* Byte 0 */
    SD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
    SD_csd->Reserved1 = CSD_Tab[0] & 0x03;
    SD_csd->TAAC = CSD_Tab[1]; /* Byte 1 */
    SD_csd->NSAC = CSD_Tab[2]; /* Byte 2 */
    SD_csd->MaxBusClkFrec = CSD_Tab[3]; /* Byte 3 */
    SD_csd->CardComdClasses = CSD_Tab[4] << 4; /* Byte 4 */
    SD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4; /* Byte 5 */
    SD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;
    SD_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7; /* Byte 6 */
    SD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
    SD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
    SD_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
    SD_csd->Reserved2 = (CSD_Tab[6] & 0x0C) >> 2;
    if (SD_csd->CSDStruct == 0)
    { /* v1 */
        SD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10; /* DeviceSize has 12 bits here */
        SD_csd->DeviceSize |= CSD_Tab[7] << 2; /* Byte 7 */
        SD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6; /* Byte 8 */
        SD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
        SD_csd->MaxRdCurrentVDDMax = CSD_Tab[8] & 0x07;
        SD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5; /* Byte 9 */
        SD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
        SD_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
        SD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7; /* Byte 10 */
    }
    else
    { /* v2 */
        SD_csd->Reserved5 = (CSD_Tab[6] & 0x03) << 2;
        SD_csd->Reserved5 |= (CSD_Tab[7] & 0xC0) >> 6; /* Byte 7 */
        SD_csd->DeviceSize = (CSD_Tab[7] & 0x3F) << 16; /* DeviceSize has 22 bits here */
        SD_csd->DeviceSize |= CSD_Tab[8] << 8; /* Byte 8 */
        SD_csd->DeviceSize |= CSD_Tab[9]; /* Byte 9 */
        SD_csd->Reserved6 = (CSD_Tab[10] & 0x80) >> 7; /* Byte 10 */
    }
    SD_csd->EraseBlockEnable = (CSD_Tab[10] & 0x40) >> 6;
    SD_csd->EraseSectorSize = (CSD_Tab[10] & 0x3F) << 1;
    SD_csd->EraseSectorSize |= (CSD_Tab[11] & 0x80) >> 7; /* Byte 11 */
    SD_csd->WrProtectGrSize = CSD_Tab[11] & 0x7F;
    SD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7; /* Byte 12 */
    SD_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
    SD_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
    SD_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
    SD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6; /* Byte 13 */
    SD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
    SD_csd->Reserved3 = CSD_Tab[13] & 0x1E;
    SD_csd->ContentProtectAppli = CSD_Tab[13] & 0x01;
    SD_csd->FileFormatGroup = (CSD_Tab[14] & 0x80) >> 7; /* Byte 14 */
    SD_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
    SD_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
    SD_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
    SD_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
    SD_csd->ECC = CSD_Tab[14] & 0x03;
    SD_csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1; /* Byte 15 */
    SD_csd->Reserved4 = CSD_Tab[15] & 0x01;

    return state;
}

SD_Error SD_GetCIDRegister(SD_SPI_Handle *sd, SD_CID *SD_cid)
{
    SD_Error state;
    uint8_t CID_Tab[16];

    state = SD_WaitReady(sd); /* make sure card is ready before we go further... */
    if (state != SD_RESPONSE_NO_ERROR)
        return SD_RESPONSE_FAILURE;

    /* request CID register (send CMD10)... */
    state = SD_SendCmd(sd, SD_CMD_SEND_CID, 0x00000000, 0xFF);
    if (state != SD_RESPONSE_NO_ERROR)
        return SD_RESPONSE_FAILURE;
    state = SD_ReceiveData(sd, CID_Tab, 16); /* receive CID register data */

    SD_cid->ManufacturerID = CID_Tab[0]; /* Byte 0 */
    SD_cid->OEM_AppliID = CID_Tab[1] << 8; /* Byte 1 */
    SD_cid->OEM_AppliID |= CID_Tab[2]; /* Byte 2 */
    SD_cid->ProdName1 = CID_Tab[3] << 24; /* Byte 3 */
    SD_cid->ProdName1 |= CID_Tab[4] << 16; /* Byte 4 */
    SD_cid->ProdName1 |= CID_Tab[5] << 8; /* Byte 5 */
    SD_cid->ProdName1 |= CID_Tab[6]; /* Byte 6 */
    SD_cid->ProdName2 = CID_Tab[7]; /* Byte 7 */
    SD_cid->ProdRev = CID_Tab[8]; /* Byte 8 */
    SD_cid->ProdSN = CID_Tab[9] << 24; /* Byte 9 */
    SD_cid->ProdSN |= CID_Tab[10] << 16; /* Byte 10 */
    SD_cid->ProdSN |= CID_Tab[11] << 8; /* Byte 11 */
    SD_cid->ProdSN |= CID_Tab[12]; /* Byte 12 */
    SD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4; /* Byte 13 */
    SD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;
    SD_cid->ManufactDate |= CID_Tab[14]; /* Byte 14 */
    SD_cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1; /* Byte 15 */
    SD_cid->Reserved2 = 1;

    return state;
}

SD_Error SD_GetSCRRegister(SD_SPI_Handle *sd, SD_SCR *SD_scr)
{
    SD_Error state;
    uint8_t SCR_Tab[8];

    if (sd->card_type == SD_Card_MMC)
    {
        printf("SCR Register is not available for MMC cards\n");
        return SD_ILLEGAL_COMMAND;
    }

    state = SD_WaitReady(sd); /* make sure card is ready before we go further... */
    if (state != SD_RESPONSE_NO_ERROR)
        return SD_RESPONSE_FAILURE;

    /* request SCR register (send ACMD51)... */
    state = SD_SendCmd(sd, SD_CMD_SEND_APP, 0x00000000, 0x65);
    if (state == SD_RESPONSE_NO_ERROR)
        state = SD_SendCmd(sd, SD_ACMD_SEND_SCR, 0x00000000, 0xFF);
    if (state != SD_RESPONSE_NO_ERROR)
        return SD_RESPONSE_FAILURE;
    state = SD_ReceiveData(sd, SCR_Tab, 8); /* receive SCR register data */

    SD_scr->SCR_Version = (SCR_Tab[0] & 0xF0) >> 4; /* Byte 0 */
    SD_scr->SpecVersion = SCR_Tab[0] & 0x0F;
    SD_scr->StateAfterErase = (SCR_Tab[1] & 0x80) >> 7; /* Byte 1 */
    SD_scr->Security = (SCR_Tab[1] & 0x70) >> 4;
    SD_scr->BusWidth = SCR_Tab[1] & 0x0F;
    SD_scr->SpecVersion3 = (SCR_Tab[2] & 0x80) >> 7; /* Byte 2 */
    SD_scr->ExSecurity = (SCR_Tab[2] & 0x78) >> 3;
    SD_scr->Reserved1 = (SCR_Tab[2] & 0x07) << 6;
    SD_scr->Reserved1 |= (SCR_Tab[3] & 0xFC) >> 2; /* Byte 3 */
    SD_scr->CmdSupport1 = (SCR_Tab[3] & 0x02) >> 1;
    SD_scr->CmdSupport2 = SCR_Tab[3] & 0x01;
    SD_scr->Reserved2 = SCR_Tab[4] << 24; /* Byte 4 */
    SD_scr->Reserved2 |= SCR_Tab[5] << 16; /* Byte 5 */
    SD_scr->Reserved2 |= SCR_Tab[6] << 8; /* Byte 6 */
    SD_scr->Reserved2 |= SCR_Tab[7]; /* Byte 7 */

    return state;
}

/**
 * @brief  Put SD in Idle state.
 * @param  None
 * @retval The SD init result
 */
static SD_InitResult SD_GoIdleState(SD_SPI_Handle *sd)
{
    uint32_t res = 0;
    uint8_t state;
    uint32_t start_time;

    SD_Bus_Hold(sd);

    state = SD_WaitReady(sd);

    /* Software reset */
    uint8_t ready_check_counter = 16;
    start_time = HAL_GetTick();
    do
    {
        state = SD_SendCmd(sd, SD_CMD_GO_IDLE_STATE, 0x00000000, 0x95);          // valid CRC is mandatory here
        if (state == 0x00 && --ready_check_counter == 0)
        {
            /* Check OCR */
            /* request OCR register (send CMD58)... */
            state = SD_SendCmd(sd, SD_CMD_READ_OCR, 0x00000000, 0xFF);
            if (state == SD_RESPONSE_NO_ERROR)
            { /* get OCR register (R3 response) and check its CCS (bit 30) */
                SD_GetResponse4b(sd, (uint8_t*) &res);
                sd->card_type = (res & 0x40000000) ? SD_Card_SDHC : SD_Card_SDSC_v2;
            }
            else
                sd->card_type = SD_Card_SDSC_v2;

            return SD_INIT_SUCESS;
        }

    } while (state != SD_IN_IDLE_STATE && (HAL_GetTick() - start_time) <= 1000);

    if (state != SD_IN_IDLE_STATE)
        return SD_INIT_NO_CARD;

    state = SD_WaitReady(sd);

    /* Voltage check*/
    state = SD_SendCmd(sd, SD_CMD_SEND_IF_COND, 0x000001AA, 0x87);          // valid CRC is mandatory here
    if (state == SD_IN_IDLE_STATE)          // SDv2
    {
        SD_GetResponse4b(sd, (uint8_t*) &res);
        if ((res & 0xFFF) != 0x01AA)
            return SD_INIT_VOLTAGE_ERROR;

        SD_WaitReady(sd);

        start_time = HAL_GetTick();
        do
        {
            state = SD_SendCmd(sd, SD_CMD_SEND_APP, 0x00, 0xFF);

            if (state != SD_IN_IDLE_STATE)
                continue;

            state = SD_SendCmd(sd, SD_ACMD_ACTIVATE_INIT, 0x40000000, 0xFF);
            SD_GetResponse4b(sd, (uint8_t*) &res);

            SD_WaitReady(sd);
        } while ((state & SD_IN_IDLE_STATE) != 0x00 && (HAL_GetTick() - start_time) <= 1000);

        if ((state & SD_IN_IDLE_STATE) != 0x00)
            return SD_INIT_APP_INIT_FAILED;

        /* Check OCR */
        /* request OCR register (send CMD58)... */
        state = SD_SendCmd(sd, SD_CMD_READ_OCR, 0x00000000, 0xFF);
        if (state == SD_RESPONSE_NO_ERROR)
        { /* get OCR register (R3 response) and check its CCS (bit 30) */
            SD_GetResponse4b(sd, (uint8_t*) &res);
            sd->card_type = (res & 0x40000000) ? SD_Card_SDHC : SD_Card_SDSC_v2;
        }
        else
            sd->card_type = SD_Card_SDSC_v2;
    }
    else          // SDv1
    {
        sd->card_type = SD_Card_SDSC_v1;

        start_time = HAL_GetTick();
        do
        {
            state = SD_SendCmd(sd, SD_CMD_SEND_APP, 0x00, 0xFF);

            if (state != SD_IN_IDLE_STATE)
                continue;

            state = SD_SendCmd(sd, SD_ACMD_ACTIVATE_INIT, 0x00000000, 0xFF);

            SD_GetResponse4b(sd, (uint8_t*) &res);

        } while ((state & SD_IN_IDLE_STATE) != 0x00 && (HAL_GetTick() - start_time) <= 1000);

        if ((state & SD_IN_IDLE_STATE) != 0x00)
        {
            do
            {
                state = SD_SendCmd(sd, SD_CMD_SEND_OP_COND, 0x00, 0xFF);
                sd->card_type = SD_Card_MMC;

            } while ((state & SD_IN_IDLE_STATE) != 0x00 && (HAL_GetTick() - start_time) <= 1000);
        }

        if ((state & SD_IN_IDLE_STATE) != 0x00)
            return SD_INIT_UNKNOW_CARD;

    }

    state = SD_WaitReady(sd);

    return SD_INIT_SUCESS;
}

/**
 * @brief  Set SD Card sector size to SD_CMD_SET_BLOCKLEN (512 bytes)
 * @param  New sector size
 * @retval The SD Response:
 *         - SD_RESPONSE_FAILURE: Sequence failed
 *         - SD_RESPONSE_NO_ERROR: Sequence succeed
 */
static SD_Error SD_FixSectorSize(SD_SPI_Handle *sd, uint16_t ssize)
{
    return SD_SendCmd(sd, SD_CMD_SET_BLOCKLEN, (uint32_t) ssize, 0xFF);
}

SD_InitResult SD_Init(SD_SPI_Handle *sd)
{
    SD_InitResult state;
    uint32_t i = 0;

    /* step 0:
     * Check if SD card is present... */

    /* step 1 :
     * reduce spi baudrate.
     */

    SPI_HandleTypeDef *hspi = sd->init.hspi;

    SPI_InitTypeDef spi_init_backup = hspi->Init;

    hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    if (HAL_SPI_Init(hspi) != HAL_OK)
    {
        // TODO: Handle error
    }

    /* step 2:
     * Card is now powered up (i.e. 1ms at least elapsed at 0.5V),
     * Supply rump up time (set MOSI HIGH) to let voltage reach stable 2.2V at least.
     * According to the specs it must be 74 SPI clock cycles minimum at 100-400Khz
     * At 25Mhz it'll be 250 times more cycles => send 2500 times 0xFF byte.
     * Chip Select pin should be set HIGH too. */

    /* set SD chip select pin high */
    HAL_GPIO_WritePin(sd->init.CS_Port, sd->init.CS_Pin, GPIO_PIN_SET);
    /* send dummy byte 0xFF (rise MOSI high for 2500*8 SPI bus clock cycles) */
    while (i++ < SD_NUM_TRIES_RUMPUP)
        SD_WriteByte(sd, SD_DUMMY_BYTE);

    /* step 3:
     * Put SD in SPI mode & perform soft reset */
    state = SD_GoIdleState(sd);

    /* step 4:
     * Force sector size to SD_BLOCK_SIZE (i.e. 512 bytes) */
    if (state == SD_INIT_SUCESS && sd->card_type != SD_Card_SDHC)
        SD_FixSectorSize(sd, (uint16_t) SD_BLOCK_SIZE);

    /* step 5:
     * Release SPI bus for other devices */
    SD_Bus_Release(sd);

    hspi->Init = spi_init_backup;
    hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    if (HAL_SPI_Init(hspi) != HAL_OK)
    {
        // TODO: Handle error
    }

    return state;
}

SD_Error SD_SectorRead(SD_SPI_Handle *sd, uint32_t readAddr, uint8_t *pBuffer)
{

    SD_Error state;

    /* non High Capacity cards use byte-oriented addresses */
    if (sd->card_type != SD_Card_SDHC)
        readAddr <<= 9;

    SD_Bus_Hold(sd); /* hold SPI bus... */

    state = SD_WaitReady(sd); /* make sure card is ready before we go further... */

    /* send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
    state = SD_SendCmd(sd, SD_CMD_READ_SINGLE_BLOCK, readAddr, 0x00);

    /* receive data if command acknowledged... */
    if (state == SD_RESPONSE_NO_ERROR)
    {
        /* read the block */
        state = SD_ReceiveData(sd, pBuffer, SD_BLOCK_SIZE);
    }

    state = SD_WaitReady(sd);

    SD_Bus_Release(sd); /* release SPI bus... */

    return state;
}

SD_Error SD_SectorWrite(SD_SPI_Handle *sd, uint32_t writeAddr, const uint8_t *pBuffer)
{
    SD_Error state;
    SD_DataResponse res;

    /* non High Capacity cards use byte-oriented addresses */
    if (sd->card_type != SD_Card_SDHC)
        writeAddr <<= 9;

    SD_Bus_Hold(sd);

    state = SD_WaitReady(sd);

    /* send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write single block */
    state = SD_SendCmd(sd, SD_CMD_WRITE_SINGLE_BLOCK, writeAddr, 0xFF);
    if (state == SD_RESPONSE_NO_ERROR)
    { /* wait at least 8 clock cycles (send >=1 0xFF bytes) before transmission starts */
        SD_ReadByte(sd);
        SD_ReadByte(sd);
        SD_ReadByte(sd);
        /* send data token to signify the start of data transmission... */
        SD_WriteByte(sd, SD_DATA_SINGLE_BLOCK_WRITE_START); /* 0xFE */
        /* send data... */

        HAL_SPI_Transmit(sd->init.hspi, pBuffer, SD_BLOCK_SIZE, HAL_MAX_DELAY);

        /* put 2 CRC bytes (not really needed by us, but required by SD) */
        SD_WriteByte(sd, 0xFF);
        SD_WriteByte(sd, 0xFF);

        /* check data response... */
        res = (SD_DataResponse) (SD_ReadByte(sd) & SD_RESPONSE_MASK); /* mask unused bits */
        if ((res & SD_RESPONSE_ACCEPTED) != 0)
        { /* card is now processing data and goes to BUSY mode, wait until it finishes... */
            state = SD_WaitBytesWritten(sd); /* make sure card is ready before we go further... */
        }
        else
            state = SD_RESPONSE_FAILURE;
    }

    SD_Bus_Release(sd); /* release SPI bus... */

    return state;
}
