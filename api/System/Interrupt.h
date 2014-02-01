/*
 * Copyright (c) 2014, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <PICoo.h>
#include <System/System_Defs.h>

namespace IO {
    class Pin;
}

typedef void (* FNIMGJMP) (void);
typedef void (*isrFunc)(void);

#pragma pack(push,2)
typedef struct {
    uint32_t    cbBlRamHeader;
    uint32_t    rcon;
} RAM_HEADER_INFO;
#pragma pack(pop)

#pragma pack(push,2)
typedef struct {
    uint32_t  cbHeader;
    uint32_t  verBootloader;
    uint32_t  verMPIDE;
    uint32_t  bootloaderCapabilities;
    uint16_t  vend;
    uint16_t  prod;
    uint32_t  imageType;
    FNIMGJMP  pJumpAddr;
    uint32_t  pProgramFlash;
    uint32_t  cbProgramFlash;
    uint32_t  pEEProm;
    uint32_t  cbEEProm;
    uint32_t  pConfig;
    uint32_t  cbConfig;
    RAM_HEADER_INFO * pRamHeader;
    uint32_t  cbRamHeader;
    uint32_t  cbBlPreservedRam;
    uint32_t  pOrgVector0;
    uint32_t  pIndirectVector0;
    uint32_t  cbVectorSpacing;
} IMAGE_HEADER_INFO;
#pragma pack(pop)

extern const IMAGE_HEADER_INFO _image_header_info;
extern const uint32_t _GEN_EXCPT_ADDR;

extern isrFunc _isr_table[NUM_INT_REQUEST];
extern thread ISRThread;

class Interrupt {
    public:
        static void __attribute__((nomips16)) restore(uint32_t);
        static uint32_t __attribute__((nomips16)) enable();
        static uint32_t __attribute__((nomips16)) disable();
        static void __attribute__((nomips16)) enableMultiVector();
        static void __attribute__((nomips16)) enableSingleVector();
        static void initializeVectorTable();
        static void setPriority(uint32_t, uint32_t, uint32_t);
        static isrFunc setVector(uint32_t, isrFunc);
        static isrFunc clearVector(uint32_t);
        static isrFunc getVector(uint32_t);
        static uint32_t enableIRQ(uint32_t);
        static uint32_t disableIRQ(uint32_t);
        static void clearFlag(uint32_t);
        static uint32_t getFlag(uint32_t);
        static uint8_t isEnabled(uint32_t);
        static void executeInterrupt(uint32_t i);
        static void attachInterrupt(uint32_t i, isrFunc f);
        static void detatchInterrupt(uint32_t i);

        static void setIndicatorPin(IO::Pin& p);
        static void indicateOn();
        static void indicateOff();
        static uint32_t getVectorByIRQ(uint32_t irq);

};

#endif
