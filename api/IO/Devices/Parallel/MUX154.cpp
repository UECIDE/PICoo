/*
 * Copyright (c) 2014, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 * 
 *  3. Neither the name of Majenko Technologies nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without 
 *     specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <PICoo.h>

namespace IO {

MUX154::MUX154(Pin& a0, Pin& a1, Pin& a2, Pin& a3, Pin& e0) {
    _a0 = &a0;
    _a1 = &a1;
    _a2 = &a2;
    _a3 = &a3;
    _e0 = &e0;

    _a0->setMode(IO::OUTPUT, IO::LOW);
    _a1->setMode(IO::OUTPUT, IO::LOW);
    _a2->setMode(IO::OUTPUT, IO::LOW);
    _a3->setMode(IO::OUTPUT, IO::LOW);
    _e0->setMode(IO::OUTPUT, IO::HIGH);
    _selected = -1;
}

void MUX154::write(uint16_t p, uint8_t l) {
    if (p > 15) {
        return;
    }
    if (l == IO::LOW) {
        _selected = p;
    } else {
        _selected = -1;
    }
    update();
}

uint8_t MUX154::read(uint16_t p) {
    return IO::LOW;
}

void MUX154::setMode(uint16_t p, uint8_t m, uint8_t d) {
}

void MUX154::update() {
    _e0->write(IO::HIGH);
    if (_selected > 15) {
        return;
    }

    _a0->write(_selected & 0x01 ? IO::HIGH : IO::LOW);
    _a1->write(_selected & 0x02 ? IO::HIGH : IO::LOW);
    _a2->write(_selected & 0x04 ? IO::HIGH : IO::LOW);
    _a3->write(_selected & 0x08 ? IO::HIGH : IO::LOW);
    _e0->write(IO::LOW);
}

}
