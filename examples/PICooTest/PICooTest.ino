#include <stdio.h>

IO::CoreIO Core;
IO::Pin LED(Core, 13, IO::OUTPUT, IO::LOW);

const char states[] = {'Z', 'R', 'S', 'H', 'M'};

thread blnk;
thread topscreen;

void setup() {
	blnk = Thread::create("blink",blinker, 0, 512);
	topscreen = Thread::create("top",top, 0, 512);
	printf("\e[2J");
}

uint32_t spoon;

void blinker(uint32_t x) {
	while(1) {
		LED.write(IO::HIGH);
		Thread::sleep(100);
		LED.write(IO::LOW);
		Thread::sleep(500);
	}
}

extern thread ThreadList;
extern thread currentThread;

void top(uint32_t x) {
	while(1) {
	    printf("\e[0;0H");
	    printf("Uptime: %lums\n", Thread::milliseconds());
	    printf("Entry    Thread     Stack %%CPU S A\n");
	    for (thread scan = ThreadList; scan; scan = scan->next) {
	        printf("%08X %-10s %-4d %4d%% %c %c\n",
	            scan->entry,
	            scan->name, (scan->stack_head - scan->sp) * 4, 
	            Thread::runtime(scan) * 100 / Thread::milliseconds(),
	            states[scan->state],
	            scan == currentThread? '*' : ' '
	        );
	    }
		Thread::sleep(100);
	}
}
