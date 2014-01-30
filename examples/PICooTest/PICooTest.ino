#include <stdio.h>

IO::CoreIO Core;
IO::Pin LED(Core, 13, IO::OUTPUT, IO::LOW);

const char states[] = {'Z', 'R', 'S', 'H', 'M'};

thread blnk;
thread topscreen;

void setup() {
	blnk = Thread::Create("blink",blinker, 0, 512);
	topscreen = Thread::Create("top",top, 0, 512);
	printf("\e[2J");
}

uint32_t spoon;

void blinker(uint32_t x) {
	while(1) {
		LED.write(IO::HIGH);
		Thread::Sleep(100);
		LED.write(IO::LOW);
		Thread::Sleep(500);
	}
}

extern thread ThreadList;
extern thread currentThread;

void top(uint32_t x) {
	while(1) {
	    printf("\e[0;0H");
	    printf("Uptime: %lums\n", Thread::Milliseconds());
	    printf("Entry    Thread     Stack %%CPU S A\n");
	    for (thread scan = ThreadList; scan; scan = scan->next) {
	        printf("%08X %-10s %-4d %4d%% %c %c\n",
	            scan->entry,
	            scan->name, (scan->stack_head - scan->sp) * 4, 
	            Thread::Runtime(scan) * 100 / Thread::Milliseconds(),
	            states[scan->state],
	            scan == currentThread? '*' : ' '
	        );
	    }
		Thread::Sleep(100);
	}
}
