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

#include <PICoo.h>

//#ifndef OPT_BOARD_DATA
//#define OPT_BOARD_DATA
//#include <Board_Data.c>
//#endif

uint32_t _caddr;

namespace IO {

    FastCorePin::FastCorePin(uint16_t pin, uint8_t mode, uint8_t data) {
        _pin = pin;
        _mode = mode;

        volatile p32_ioport *iop;
        uint8_t port;


        if (pin >= NUM_DIGITAL_PINS_EXTENDED) {
            return;
        }

        if ((port = digitalPinToPort(pin)) == NOT_A_PIN) {
            return;
        }

        _control.base = (uint32_t)portRegisters(port);
        _control.bit = digitalPinToBitMask(pin);

        setMode(mode);
    }

    void FastCorePin::write(uint8_t level) {
        asm volatile("addiu $t1, $a0, 8");
        asm volatile("lw $t5, 0($t1)"); // tris
        asm volatile("beqz $a1, _off");
        asm volatile("lw $t4, 4($t1)"); // Bitmask
        asm volatile("sw $t4, 40($t5)");
        asm volatile("jr $ra");
        asm volatile("_off: sw $t4, 36($t5)");
        asm volatile("jr $ra");
    }

    uint8_t FastCorePin::read() {
    }

    void FastCorePin::setMode(uint8_t mode, uint8_t data) {
        uint32_t                bit;
        uint8_t                 port;
        volatile p32_ioport *   iop;
        uint8_t                 timer;
    #if !defined(__PIC32MX1XX__) && !defined(__PIC32MX2XX__)
        uint32_t                cn;
    #endif

        /* Check if pin number is in valid range.
        */
        if (_pin >= NUM_DIGITAL_PINS_EXTENDED)
        {
            return;
        }

    #if (OPT_BOARD_DIGITAL_IO != 0)
        /* Peform any board specific processing.
        */
        int _board_pinMode(uint8_t pin, uint8_t mode)

        if (_board_pinMode(_pin, mode) != 0)
        {
            return;
        }
    #endif  // OPT_BOARD_DIGITAL_IO

        //* Get the port number for this pin.
        if ((port = digitalPinToPort(_pin)) == NOT_A_PIN)
        {
            return;
        }

        //* Obtain pointer to the registers for this io port.
        iop = (p32_ioport *)portRegisters(port);

        //* Obtain bit mask for the specific bit for this pin.
        bit = digitalPinToBitMask(_pin);

    #if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
        volatile uint32_t * pps;

        // The MX1xx/MX2xx support peripheral pin select (PPS). It is necessary
        // to ensure that the pin is mapped as a general purpose i/o and not
        // mapped to a peripheral.
        if (isPpsPin(_pin))
        {
            pps = ppsOutputRegister(_pin);
            *pps = ppsOutputSelect(PPS_OUT_GPIO);
        }

        // The MX1xx/MX2xx devices have an ANSELx register associated with
        // each io port that is used to control analog/digital mode of the
        // analog input capable pins.
        // Clear the bit in the ANSELx register to ensure that the pin is in
        // digital input mode.
        iop->ansel.clr = bit;
   #else
        if (port == _IOPORT_PB)
        {
            //* The MX3xx-MX7xx PIC32 devices have all of the analog capable pins on
            //  PORTB. If this is a PORTB pin, we have to set it to digital mode.
            //  You have to set the bit in the AD1PCFG for an analog pin to be used as a
            //  digital input. They come up after reset as analog input with the digital
            //  input disabled. For the PORTB pins you switch between analog input and
            //  digital input using AD1PCFG.

            AD1PCFGSET = bit;

        }
    #endif  // defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)


        switch (mode) {
            case IO::INPUT:
                //* Determine if this is an output compare pin. If so,
                //* we need to make sure PWM output is off.

// TODO: Adapt this to the proper PWM system when it's written
//                timer = digitalPinToTimerOC(_pin) >> _BN_TIMER_OC;
//                if (timer != NOT_ON_TIMER)
//                {
//                    turnOffPWM(timer);
//                }

                if (data == IO::PULLUP) {
        #if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
                    iop->cnpu.set = bit;
                    iop->cnpd.clr = bit;
        #else
                    cn = digitalPinToCN(_pin);
                    if (cn != NOT_CN_PIN) {
                        CNPUESET = cn;
                    }
        #endif
                } else if(data == IO::PULLDOWN) {
        #if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
                    iop->cnpu.clr = bit;
                    iop->cnpd.set = bit;
        #else
                    cn = digitalPinToCN(_pin);
                    if (cn != NOT_CN_PIN) {
                        CNPUECLR = cn;
                    }
        #endif
                } else {
        #if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
                    iop->cnpu.clr = bit;
                    iop->cnpd.clr = bit;
        #else
                    cn = digitalPinToCN(_pin);
                    if (cn != NOT_CN_PIN) {
                        CNPUECLR = cn;
                    }
        #endif
                }
               //* May  1, 2011
                //* according to item #26 in PIC32MX5XX-6XX-7XX Errata.pdf
                //* if we are setting to input, set the data bit to zero first
                iop->lat.clr  = bit;    //clear to output bit
                iop->tris.set = bit;    //make the pin an input
                break;

            case IO::OUTPUT:
                iop->tris.clr = bit;
                if (data) {
                    iop->lat.set = bit;
                } else {
                    iop->lat.clr = bit;
                }
                break;
        }

    }

}
