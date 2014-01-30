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

#include <System/System_Defs.h>

typedef void (* FNIMGJMP) (void);
typedef void (*isrFunc)(void);

#pragma pack(push,2)
typedef struct {
    uint32_t    cbBlRamHeader;     // the number of bytes of this header as written by the bootloader
    uint32_t    rcon;              // value of RCON before the bootloader clears it
} RAM_HEADER_INFO;
#pragma pack(pop)

#pragma pack(push,2)
typedef struct {
    uint32_t  cbHeader;               // length of this structure
    uint32_t  verBootloader;          // version of the booloader that loaded the sketch, it will be 0xFFFFFFFF if the bootloader did not write the version.
    uint32_t  verMPIDE;               // the version number of MPIDE that build the sketch
    uint32_t  bootloaderCapabilities; // capabilities of the bootloader defined by the blCapXXX bits.
    uint16_t  vend;                   // vendor ID as assigned by the bootloader, 0xFF if undefined
    uint16_t  prod;                   // product ID as assigned by the bootloader, 0xFF if undefined, or 0xFE is unassigned
    uint32_t  imageType;              // see image bit field definition above
    FNIMGJMP  pJumpAddr;              // the execution address that the bootloader will jump to
    uint32_t  pProgramFlash;          // also known as base address, that is, the first byte of program flash used by the sketch
    uint32_t  cbProgramFlash;         // the number of bytes of flash used by the sketch as defined by the linker script
    uint32_t  pEEProm;                // pointer to the eeprom area, usually at the end of flash, but now can be defined by the linker script
    uint32_t  cbEEProm;               // the length of eeprom, usually 4K but can now be defined by the linker script
    uint32_t  pConfig;                // physical address pointer to the config bits
    uint32_t  cbConfig;               // length of config bits.
    RAM_HEADER_INFO * pRamHeader;     // pointer to the ram header
    uint32_t  cbRamHeader;            // length of the ram header as specified by the linker and will be cleared/used by the bootloader
    uint32_t  cbBlPreservedRam;       // the amount RAM the bootloader will not touch, 0xA0000000 -> 0xA0000000 + cbBlPerservedRam; Debug data, Ram Header and Persistent data must be in this section
    uint32_t  pOrgVector0;            // A pointer to the compiler generated vector 0 in the execption memory
    uint32_t  pIndirectVector0;       // A pointer to the indirect jump vector 0 required to resolve conflicting peripherals
    uint32_t  cbVectorSpacing;        // the number of bytes between the vector table entries, applies to both org/new
} IMAGE_HEADER_INFO;
#pragma pack(pop)

extern const IMAGE_HEADER_INFO _image_header_info;
extern const uint32_t _GEN_EXCPT_ADDR;

namespace Interrupt {
    extern void __attribute__((nomips16)) Restore(uint32_t);
    extern uint32_t __attribute__((nomips16)) Enable();
    extern uint32_t __attribute__((nomips16)) Disable();
    extern void __attribute__((nomips16)) EnableMultiVector();
    extern void __attribute__((nomips16)) EnableSingleVector();
    extern void InitializeVectorTable();
    extern void SetPriority(uint32_t, uint32_t, uint32_t);
    extern isrFunc SetVector(uint32_t, isrFunc);
    extern isrFunc GetVector(uint32_t);
    extern uint32_t EnableIRQ(uint32_t);
    extern uint32_t DisableIRQ(uint32_t);
    extern void ClearFlag(uint32_t);
    extern uint32_t GetFlag(uint32_t);
    extern uint8_t isEnabled(uint32_t);
    extern void ExecuteInterrupt(uint32_t i);
    extern void AttachInterrupt(uint32_t i, isrFunc f);
    extern void DetatchInterrupt(uint32_t i);
}

#endif
