// We have 2 LEDs on most chipKIT boards - let's make use of them.
IO::CoreIO Core;
IO::Pin LED1(Core, PIN_LED1, IO::OUTPUT, IO::LOW);
IO::Pin LED2(Core, PIN_LED2, IO::OUTPUT, IO::LOW);

// We want to output data on UART1.  Let's make it Serial for familiarity.
UART1 Serial;

// Our "TOP" screen needs some translations...
const char states[] = {'Z', 'R', 'S', 'H', 'M', 'W'};

// Two threads and one semaphore.
thread blnk;
thread topscreen;
semaphore sem;

// Let's gain access to the internals of the thread system.
extern thread ThreadList; // Linked list of threads
extern thread currentThread;  // Currently active thread

void setup() {
	// Let's make one of the LEDs flicker as interrupts occur.  That means a glow
	// for the high speed interrupts of the context switcher.
	Interrupt::SetIndicatorPin(LED2);

	// Set up our serial and clear the screen
	Serial.begin(115200);
	Serial.print("\e[2J");

	// Create our two threads.  We only need a small stack for each instead of the default.
	blnk = Thread::Create("blink",blinker, 0, 128);
	topscreen = Thread::Create("top",top, 0, 512);
}

// This is our blinker routine.  Flash an LED over a 500ms period in total.
// The while(1) is needed or the thread will terminate when it reaches the end.
void blinker(uint32_t x) {
	while(1) {
		// Indicate to the other thread through our semaphore that we are blinking the LED
		Thread::Signal(sem);
		LED1.write(IO::HIGH);
		Thread::Sleep(100);
		LED1.write(IO::LOW);
		Thread::Sleep(400);
	}
}

// And this is our "TOP" screen.
void top(uint32_t x) {
	int lastCharacter = -1;
	while(1) {
		uint32_t su = 0;
		uint32_t sa = 0;
		// Stop here until we get the signal.
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
	            scan->name, Math::MulU(scan->stack_head - scan->sp, 4), 
	            Math::DivU(Math::MulU(Thread::Runtime(scan), 100), Thread::Milliseconds()),
	            states[scan->state],
	            scan == currentThread? '*' : ' '
	        );
	        sa += scan->stack_size;
	        su += Math::MulU(scan->stack_head - scan->sp, 4);
	    }
	    Serial.printf("Total stack size: %u, Allocated: %u, Used: %u\n", STACK_SIZE, sa, su);
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
	}
}
