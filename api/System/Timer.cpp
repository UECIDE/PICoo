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

const uint8_t Timer::_priorityList[5] = {5, 4, 3, 2, 1};
uint8_t Timer::_alloc = 0;

Timer::Timer(uint8_t allowed) {
    _timerNumber = 0;
    for (int i = 0; i < 5; i++) {
        int b = 1<<(_priorityList[i] - 1);
        if ((_alloc & b) == 0) {
            if (allowed & b) {
                _alloc |= b;
                _timerNumber = _priorityList[i];
                break;
            }
        }
    }
}

uint32_t Timer::getIRQ() {
    switch (_timerNumber) {
        case 1:
            return _TIMER_1_IRQ;
        case 2:
            return _TIMER_2_IRQ;
        case 3:
            return _TIMER_3_IRQ;
        case 4:
            return _TIMER_4_IRQ;
        case 5:
            return _TIMER_5_IRQ;
    }
    return 0;
}

uint32_t Timer::getNumber() {
    return _timerNumber;
}

void Timer::attachInterrupt(isrFunc f) {
    uint32_t irq = getIRQ();
    if (irq > 0) {
        Interrupt::attachInterrupt(irq, f);
        Interrupt::enableIRQ(irq);
    }
}

void Timer::detatchInterrupt() {
    uint32_t irq = getIRQ();
    if (irq > 0) {
        Interrupt::disableIRQ(irq);
        Interrupt::detatchInterrupt(irq);
    }
}

void Timer::setPeriod(uint32_t us) {
}

void Timer::setFrequency(uint32_t hz) {
    uint32_t baseClock = F_CPU;
    uint8_t ps = 0;

    if (_timerNumber == 1) { // Type A timer

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 8;
            ps = 1;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 64;
            ps = 2;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 256;
            ps = 3;
        }

    } else { // Type B timer

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 2;
            ps = 1;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 4;
            ps = 2;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 8;
            ps = 3;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 16;
            ps = 4;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 32;
            ps = 5;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 64;
            ps = 6;
        }

        if (baseClock / hz > 65535) {
            baseClock = F_CPU / 256;
            ps = 7;
        }
    }

    switch (_timerNumber) {
        case 1:
            T1CONbits.TCKPS = ps;
            PR1 = baseClock / hz;
            break;
        case 2:
            T2CONbits.TCKPS = ps;
            PR2 = baseClock / hz;
            break;
        case 3:
            T3CONbits.TCKPS = ps;
            PR3 = baseClock / hz;
            break;
        case 4:
            T4CONbits.TCKPS = ps;
            PR4 = baseClock / hz;
            break;
        case 5:
            T5CONbits.TCKPS = ps;
            PR5 = baseClock / hz;
            break;
    }
}

void Timer::start() {
    switch (_timerNumber) {
        case 1:
            T1CONbits.TON=1;
            break;
        case 2:
            T2CONbits.TON=1;
            break;
        case 3:
            T3CONbits.TON=1;
            break;
        case 4:
            T4CONbits.TON=1;
            break;
        case 5:
            T5CONbits.TON=1;
            break;
    }
}

void Timer::stop() {
    switch (_timerNumber) {
        case 1:
            T1CONbits.TON=0;
            break;
        case 2:
            T2CONbits.TON=0;
            break;
        case 3:
            T3CONbits.TON=0;
            break;
        case 4:
            T4CONbits.TON=0;
            break;
        case 5:
            T5CONbits.TON=0;
            break;
    }
}

void Timer::release() {
    _alloc &= ~(1<<(_timerNumber-1));
}

uint8_t Timer::acquire() {
    uint8_t b = 1<<(_timerNumber-1);
    while (_alloc & b) {
        Thread::uSleep(1);
    }
}

