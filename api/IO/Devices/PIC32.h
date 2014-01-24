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

#ifndef _IO_PIC32_H
#define _IO_PIC32_H

/*
 * PIC32 Core digital IO device
 */

#include <PICoo.h>

#ifndef OPT_BOARD_DATA
#define OPT_BOARD_DATA
#include <Board_Data.c>
#endif

namespace IO {
    class PIC32 : public Parallel {
        public:
            void write(uint16_t pin, uint8_t level);
            uint8_t read(uint16_t pin);
            void setMode(uint16_t pin, uint8_t mode, uint8_t data = IO::OFF);

        private:
            uint32_t digitalPinToPort(uint16_t pin);
            uint32_t digitalPinToBitMask(uint16_t pin);
            p32_ioport *portRegisters(uint16_t port);

            uint8_t isPpsPin(uint16_t pin);
            volatile uint32_t *ppsOutputRegister(uint16_t pin);
            uint32_t ppsInputSelect(uint16_t pin);
            uint32_t ppsOutputSelect(uint32_t func);

            enum {
                NOT_A_PIN = 255
            };

            enum {
                NOT_PPS_PIN = 255
            };

    };
}

#endif
