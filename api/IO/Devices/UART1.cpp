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

volatile uint32_t isr_count = 0;

UART1::UART1() {
//    if (UART1::txBuffer == NULL) {
//        UART1::txBuffer = new CircularBuffer(64);
//    }
    if (UART1::rxBuffer == NULL) {
        UART1::rxBuffer = new CircularBuffer(64);
    }
}

void UART1::begin(uint32_t baud) {

    Interrupt::AttachInterrupt(_UART1_RX_IRQ, &UART1::rxtxISR);
    Interrupt::SetPriority(_UART_1_VECTOR, 5, 0);
    Interrupt::EnableIRQ(_UART1_RX_IRQ);
//    Interrupt::EnableIRQ(_UART1_TX_IRQ);

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

//void __attribute__((interrupt(),nomips16)) UART1::rxtxISR() {
void UART1::rxtxISR() {
    isr_count++;
//    IFS0CLR = 1<<27;
//    if (Interrupt::GetFlag(_UART1_RX_IRQ)) {
//        while ((U1STA & 1) != 0) {
            uint8_t ch = U1RXREG;
            UART1::rxBuffer->write(ch);
//        }
//        Interrupt::ClearFlag(_UART1_RX_IRQ);
 //   }
//
//    if (Interrupt::GetFlag(_UART1_TX_IRQ)) {
//        Interrupt::ClearFlag(_UART1_TX_IRQ);
//        if (UART1::txBuffer->available()) {
//            U1TXREG = UART1::txBuffer->read();
//        } else {
//            Interrupt::DisableIRQ(_UART1_TX_IRQ);
//        }
//    }
}

void UART1::flush() {
    while(UART1::txBuffer->available()) {
        Thread::USleep(1);
        continue;
    }
    UART1::rxBuffer->clear();
}

void UART1::write(uint8_t d) {
    while((U1STA & (1 << _UARTSTA_UTXBF)) != 0) {
        Thread::USleep(1);
    }
    U1TXREG = d;

//    while (UART1::txBuffer->available() >= 60) {
//        Thread::USleep(1);
//        continue;
//    }
//    txBuffer->write(d);
//    if (!Interrupt::isEnabled(_UART1_TX_IRQ)) {
//        U1TXREG = UART1::txBuffer->read();
//        Interrupt::EnableIRQ(_UART1_TX_IRQ);
//    }
} 

int UART1::read() {
    return UART1::rxBuffer->read();
}

int UART1::available() {
    return UART1::rxBuffer->available();
}

#endif
