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

#ifndef _PRINT_H
#define _PRINT_H

#include <PICoo.h>

class Print
{
    private:
        void printNumber(uint32_t, uint8_t);
        void printFloat(double, uint8_t);

    public:
    
        static const uint8_t DEC = 10;
        static const uint8_t HEX = 16;
        static const uint8_t OCT = 8;
        static const uint8_t OCTAL = 8;
        static const uint8_t BIN = 2;
        static const uint8_t BINARY = 2;
        static const uint8_t CHAR = 0;

        virtual void write(uint8_t) = 0;
        virtual void write(const char *str);
        virtual void write(const uint8_t *buffer, size_t size);

        //void print(const String &);
        void print(const char[]);
        void print(char, int = Print::CHAR);
        void print(uint8_t, int = Print::CHAR);
        void print(int, int = Print::DEC);
        void print(uint32_t, int = Print::DEC);
        void print(int32_t, int = Print::DEC);
        void print(double, int = 2);

        void printf(const char *fmt, ...);

        //void println(const String &s);
        void println(const char[]);
        void println(char, int = Print::CHAR);
        void println(uint8_t, int = Print::CHAR);
        void println(uint32_t, int = Print::DEC);
        void println(int32_t, int = Print::DEC);
        void println(double, int = 2);
        void println(void);
};

#endif
