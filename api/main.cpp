#include <PICoo.h>
#include <stdio.h>

extern const uint32_t __attribute__((section(".mpide_version"))) _verMPIDE_Stub = MPIDEVER; 

extern "C" {
extern void __use_isr_install(void);
__attribute__((section(".comment"))) void (*__use_force_isr_install)(void) = &__use_isr_install;
}


extern "C" {
    extern void ThreadScheduler();
    extern void restoreThreadContext();
}


extern void setup();
extern void loop();

thread IdleThread;
thread MasterThread;

extern uint32_t spoon;

void IdleThreadFunction(uint32_t x) {
    while (1) {
        for (thread scan = ThreadList; scan; scan = scan->next) {
            if (scan->state == Thread::ZOMBIE) {
                scan->sp = scan->stack_head;
            }
        }
        continue;
    }
}

void MasterThreadFunction(uint32_t x) {
    setup();
    while (1) {
        loop();
    }
}

extern thread currentThread;

const char states[] = {'Z', 'R', 'S', 'H', 'M'};

uint32_t counter;
    uint32_t t;
uint32_t spOfCurrentThread;

void __attribute__((weak)) setup() { }
void __attribute__((weak)) loop() { Thread::Terminate(); }

extern "C" {
    void _mon_putc(int c) {
        while ((U1STA & (1 << _UARTSTA_UTXBF)) != 0) {
            continue;
        }
        U1TXREG = c;
    }
}

#include <stdio.h>

int main() {
    System::Configure(F_CPU);
    Interrupt::EnableSingleVector();
    Interrupt::InitializeVectorTable();
    JTAG::Disable();
//    U1BRG = (80000000UL / 16 / 115200) - 1;
//    U1MODE = (1<<_UARTMODE_ON);
//    U1STA = (1 << _UARTSTA_UTXEN) | (1 << _UARTSTA_URXEN);

    IdleThread = Thread::Create("[idle]", IdleThreadFunction, 0, 32);
    MasterThread = Thread::Create("[arduino_api]", MasterThreadFunction);
    Thread::Start();

    while(1) {
        continue;
    }
    return 0;
}
