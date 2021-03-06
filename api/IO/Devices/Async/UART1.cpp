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

#ifdef _UART_1_VECTOR

CircularBuffer *UART1::txBuffer = NULL;
CircularBuffer *UART1::rxBuffer = NULL;

UART1::UART1() {
//    if (UART1::txBuffer == NULL) {
//        UART1::txBuffer = new CircularBuffer(64);
//    }
    if (UART1::rxBuffer == NULL) {
        UART1::rxBuffer = new CircularBuffer(64);
    }
}

void UART1::begin(uint32_t baud) {

    Interrupt::attachInterrupt(_UART1_RX_IRQ, &UART1::rxtxISR);
    Interrupt::setPriority(_UART_1_VECTOR, 3, 0);
    Interrupt::enableIRQ(_UART1_RX_IRQ);
//    Interrupt::enableIRQ(_UART1_TX_IRQ);

    if (baud > 9600) {
        U1BRG = (F_CPU / 4 / baud) - 1;
        U1MODE = (1<<_UARTMODE_ON) | (1<<_UARTMODE_BRGH);
        U1STA = (1 << _UARTSTA_UTXEN) | (1 << _UARTSTA_URXEN) | (0b10 << 14);
    } else {
        U1BRG = (F_CPU / 16 / baud) - 1;
        U1MODE = (1<<_UARTMODE_ON);
        U1STA = (1 << _UARTSTA_UTXEN) | (1 << _UARTSTA_URXEN) | (0b10 << 14);
    }
}

void UART1::rxtxISR() {
    uint8_t ch = U1RXREG;
    UART1::rxBuffer->write(ch);
}

void UART1::flush() {
    while(UART1::txBuffer->available()) {
        Thread::uSleep(1);
        continue;
    }
    UART1::rxBuffer->clear();
}

void UART1::write(uint8_t d) {
    while((U1STA & (1 << _UARTSTA_UTXBF)) != 0) {
        Thread::uSleep(1);
    }
    U1TXREG = d;
} 

int UART1::read() {
    return UART1::rxBuffer->read();
}

int UART1::available() {
    return UART1::rxBuffer->available();
}

#endif
