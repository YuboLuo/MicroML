/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*******************************************************************************
 *
 * SDCardLogMode.c
 *
 * Wakes up every 5 seconds from LPM3 to measure and store its
 * internal temperature sensor & battery monitor data to SDCard
 *
 * March 2016
 * E. Chen
 *
 ******************************************************************************/

#include "SDCardLogMode.h"

SDCardLib sdCardLib;

// FatFs Static Variables
static FIL fil;        /* File object */
static char filename[31];
static FRESULT rc;


int16_t *writeSD_data16(int16_t *buffer, char *f_name, uint16_t length){
    uint16_t bw = 0;

    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);

    //Configure SDCard CS pin
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, BIT0);
    GPIO_setAsOutputPin(GPIO_PORT_P4, BIT0);

    //Plugin SDcard interface to SDCard lib
    SDCardLib_init(&sdCardLib, &sdIntf_MSP430FR5994LP);

    strcpy(filename, f_name);

    // open a log file
    rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS); // open file

    rc = f_write(&fil, (char *) buffer, sizeof(int16_t) * length, &bw);

    f_close(&fil);

    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);

    return buffer;
}

int16_t *readSD_data16(int16_t *buffer, char *f_name, uint16_t length){

    uint16_t br = 0;

    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);

    //Configure SDCard CS pin
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, BIT0);
    GPIO_setAsOutputPin(GPIO_PORT_P4, BIT0);

    //Plugin SDcard interface to SDCard lib
    SDCardLib_init(&sdCardLib, &sdIntf_MSP430FR5994LP);

    strcpy(filename, f_name);

    // open file
    rc = f_open(&fil, filename, FA_READ | FA_OPEN_EXISTING); // open file
    if(rc){
        f_close(&fil);
        SDCardLib_unInit(&sdCardLib);
        return buffer;
    }

    rc = f_read(&fil, (char *) buffer, sizeof(int16_t) * length, &br); // read 60KB data, 16bit, it can read at maximum 65535bytes at a time
    if(rc){
        f_close(&fil);
        SDCardLib_unInit(&sdCardLib);
        return buffer;
    }

    f_close(&fil);

    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);

    return buffer;
}

