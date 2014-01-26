#include <PICoo.h>
#include <stdio.h>

extern const uint32_t __attribute__((section(".mpide_version"))) _verMPIDE_Stub = MPIDEVER; 

extern "C" {
extern void __use_isr_install(void);
__attribute__((section(".comment"))) void (*__use_force_isr_install)(void) = &__use_isr_install;
}


extern void setup();
extern void loop();

thread IdleThread;
thread MasterThread;

void IdleThreadFunction() {
    while (1) {
        printf(".");
        continue;
    }
}

void MasterThreadFunction() {
printf(">>>");
    setup();
    while (1) {
        loop();
    }
}

void __attribute__((weak)) setup() {  }
void __attribute__((weak)) loop() { printf("."); }

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
    Interrupt::EnableMultiVector();
    Interrupt::InitializeVectorTable();
    JTAG::Disable();
    U1BRG = (80000000UL / 16 / 9600) - 1;
    U1MODE = (1<<_UARTMODE_ON);
    U1STA = (1 << _UARTSTA_UTXEN) | (1 << _UARTSTA_URXEN);

    printf("Ready\n");

    printf("Creating idle thread...");
    IdleThread = Thread::Create(IdleThreadFunction, 0, 512);
    printf("%08X\n", IdleThread);
    printf("Creating master thread...");
    MasterThread = Thread::Create(MasterThreadFunction);
    printf("%08X\n", MasterThread);
    printf("Starting threads...\n");
    Thread::Start();
    printf("Threads started OK\n");

    while(1) {
        continue;
    }
    return 0;
}
