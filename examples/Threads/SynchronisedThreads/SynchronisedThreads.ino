#include <stdio.h>

IO::PIC32 Core;
IO::Pin LED(Core, 13, IO::OUTPUT, IO::LOW);

UART1 Serial;
const char states[] = {'Z', 'R', 'S', 'H', 'M', 'W'};

thread blnk;
thread topscreen;

extern uint32_t isr_count;

semaphore sem;

void setup() {
	Serial.begin(115200);
	Serial.print("\e[2J");
//	printf("\e[2J");
	blnk = Thread::Create("blink",blinker, 0, 512);
	topscreen = Thread::Create("top",top, 0, 512);
}

uint32_t spoon;

void blinker(uint32_t x) {
	while(1) {
		Thread::Signal(sem);
		LED.write(IO::HIGH);
		Thread::Sleep(10);
		LED.write(IO::LOW);
		Thread::Sleep(100);
	}
}

extern thread ThreadList;
extern thread currentThread;

void top(uint32_t x) {
	int lastCharacter = -1;
	while(1) {
		uint32_t ms = 0;
		Thread::Wait(sem);
		if (Serial.available()) {
			lastCharacter = Serial.read();
		}
	    Serial.print("\e[0;0H");
	    
	    Serial.print("Uptime: ");
	    Serial.println(Thread::Milliseconds());
	    Serial.println("Entry    Thread         Stack CPU% S A");
	    for (thread scan = ThreadList; scan; scan = scan->next) {
	        Serial.printf("%08X %-14s %-4d %4u%% %c %c\n",
	            scan->entry,
	            scan->name, (scan->stack_head - scan->sp) * 4, 
	            Math::DivU(Math::MulU(Thread::Runtime(scan), 100), Thread::Milliseconds()),
	            states[scan->state],
	            scan == currentThread? '*' : ' '
	        );
	        ms += Thread::Runtime(scan);
	    }
	    Serial.printf("Calculated ms: %u\n", ms);
	    Serial.println();
	    Serial.printf("Last character pressed: %3d (%c)\n", lastCharacter, lastCharacter >= ' ' && lastCharacter <= 'z' ? lastCharacter : '.');
	    Serial.println();
	    Serial.println("Serial RX Circular Buffer Contents:");
	    for (uint8_t x = 0; x < 64; x++) {
	    	Serial.printf("%02X ", Serial.rxBuffer->getEntry(x));
	    	if (Math::ModU(x, 16) == 15) {
	    		Serial.println();
	    	}
	    }
    	Serial.println();
    	Serial.printf("Head: %3d Tail: %3d\n", Serial.rxBuffer->getHead(), Serial.rxBuffer->getTail());
    	Serial.printf("ISR Hits: %d\n", isr_count);
	}
}
