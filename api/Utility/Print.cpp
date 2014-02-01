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

void Print::write(const char *str) {
	while (*str) {
		write(*str++);
	}
}

void Print::write(const uint8_t *buffer, size_t size) {
	while (size--) {
		write(*buffer++);
	}
}


void Print::print(const char str[]) {
	write(str);
}

void Print::print(char c, int base) {
	print((int32_t) c, base);
}

void Print::print(unsigned char b, int base) {
	print((uint32_t) b, base);
}

void Print::print(int32_t n, int base) {
	if (base == Print::CHAR) {
		write(n);
	} else if (base == Print::DEC) {
		if (n < 0) {
			print('-');
			n = -n;
		}
		printNumber(n, Print::DEC);
	} else {
		printNumber(n, base);
	}
}

void Print::print(uint32_t n, int base)
{
	if (base == Print::CHAR) {
		write(n);
	} else {
		printNumber(n, base);
	}
}

void Print::print(double n, int digits) {
	printFloat(n, digits);
}

void Print::println()
{
	print('\r');
	print('\n');
}

void Print::println(const char c[]) {
	print(c);
	println();
}

void Print::println(char c, int base) {
	print(c, base);
	println();
}

void Print::println(unsigned char b, int base) {
	print(b, base);
	println();
}

void Print::println(int32_t n, int base) {
	print(n, base);
	println();
}

void Print::println(uint32_t n, int base) {
	print(n, base);
	println();
}

void Print::println(double n, int digits) {
	print(n, digits);
	println();
}

void Print::printNumber(unsigned long n, uint8_t base)
{
    unsigned char buf[8 * sizeof(uint32_t)];
    unsigned long i = 0;

	if (n == 0) {
		print('0');
		return;
	}

	while (n > 0) {
		buf[i++] = Math::modU(n, base);
		n = Math::divU(n, base);
	}

	for (; i > 0; i--) {
		print((char) (buf[i - 1] < 10 ?
					'0' + buf[i - 1] :
					'A' + buf[i - 1] - 10));
	}
}

void Print::printFloat(double number, uint8_t digits)
{
	// Handle negative numbers
	if (number < 0.0) {
		 print('-');
		 number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i=0; i<digits; ++i)
		rounding /= 10.0;

	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	print(int_part);

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0)
		print(".");

	// Extract digits from the remainder one at a time
	while (digits-- > 0)
	{
		remainder *= 10.0;
		int toPrint = int(remainder);
		print(toPrint);
		remainder -= toPrint;
	}
}

void Print::printf(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char temp[1];
    uint32_t slen = vsnprintf(temp, 1, fmt, va);
    va_end(va);
    char out[slen + 2];
    va_start(va, fmt);
    vsnprintf(out, slen+1, fmt, va);
    va_end(va);
    print(out);
}
