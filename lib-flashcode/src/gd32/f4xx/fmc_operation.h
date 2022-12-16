/*!
    \file    fmc_operation.h
    \brief   the header file of fmc_operation 
    
    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#ifndef __FMC_OPERATION_H
#define __FMC_OPERATION_H

#include <stdio.h>
#include "gd32f4xx.h"

/* FMC sector information */
typedef struct
{
    uint32_t sector_name;                                         /*!< the name of the sector */
    uint32_t sector_num;                                          /*!< the number of the sector */
    uint32_t sector_size;                                         /*!< the size of the sector */
    uint32_t sector_start_addr;                                   /*!< the start address of the sector */
    uint32_t sector_end_addr;                                     /*!< the end address of the sector */
} fmc_sector_info_struct;

/* sector size */
#define SIZE_16KB                  ((uint32_t)0x00004000U)        /*!< size of 16KB*/
#define SIZE_64KB                  ((uint32_t)0x00010000U)        /*!< size of 64KB*/
#define SIZE_128KB                 ((uint32_t)0x00020000U)        /*!< size of 128KB*/
#define SIZE_256KB                 ((uint32_t)0x00040000U)        /*!< size of 256KB*/

/* FMC BANK address */
#define FMC_START_ADDRESS          FLASH_BASE                               /*!< FMC start address */
#define FMC_BANK0_START_ADDRESS    FMC_START_ADDRESS                        /*!< FMC BANK0 start address */
#define FMC_BANK1_START_ADDRESS    ((uint32_t)0x08100000U)                  /*!< FMC BANK1 start address */
#define FMC_SIZE                   (*(uint16_t *)0x1FFF7A22U)               /*!< FMC SIZE */
#define FMC_END_ADDRESS            (FLASH_BASE + (FMC_SIZE * 1024) - 1)     /*!< FMC end address */
#define FMC_MAX_END_ADDRESS        ((uint32_t)0x08300000U)                  /*!< FMC maximum end address */

/* FMC error message */
#define FMC_WRONG_SECTOR_NAME      ((uint32_t)0xFFFFFFFFU)        /*!< wrong sector name*/
#define FMC_WRONG_SECTOR_NUM       ((uint32_t)0xFFFFFFFFU)        /*!< wrong sector number*/
#define FMC_INVALID_SIZE           ((uint32_t)0xFFFFFFFFU)        /*!< invalid sector size*/
#define FMC_INVALID_ADDR           ((uint32_t)0xFFFFFFFFU)        /*!< invalid sector address*/

/* get the sector number, size and range of the given address */
fmc_sector_info_struct fmc_sector_info_get(uint32_t addr);
/* get the sector number by sector name */
uint32_t sector_name_to_number(uint32_t sector_name);
/* erases the sector of a given sector number */
void fmc_erase_sector_by_address(uint32_t address);
/* write 32 bit length data to a given address */
void fmc_write_32bit_data(uint32_t address, uint16_t length, int32_t* data_32);
/* read 32 bit length data from a given address */
void fmc_read_32bit_data(uint32_t address, uint16_t length, int32_t* data_32);
/* write 16 bit length data to a given address */
void fmc_write_16bit_data(uint32_t address, uint16_t length, int16_t* data_16);
/* read 16 bit length data from a given address */
void fmc_read_16bit_data(uint32_t address, uint16_t length, int16_t* data_16);
/* write 8 bit length data to a given address */
void fmc_write_8bit_data(uint32_t address, uint16_t length, int8_t* data_8);
/* read 8 bit length data from a given address */
void fmc_read_8bit_data(uint32_t address, uint16_t length, int8_t* data_8);

#endif/* __FMC_OPERATION_H */
