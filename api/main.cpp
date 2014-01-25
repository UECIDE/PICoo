#include <PICoo.h>

extern const uint32_t __attribute__((section(".mpide_version"))) _verMPIDE_Stub = MPIDEVER; 

extern "C" {
extern void __use_isr_install(void);
__attribute__((section(".comment"))) void (*__use_force_isr_install)(void) = &__use_isr_install;
}


extern void setup();
extern void loop();

uint32_t IdleThread;
uint32_t MasterThread;

void IdleThreadFunction() {
    while (1) {
        continue;
    }
}

void MasterThreadFunction() {
    setup();
    while (1) {
        loop();
    }
}

//IO::PIC32 Core;
//IO::Pin LED(Core, 13, IO::OUTPUT, IO::LOW);

void __attribute__((weak)) setup() { Thread::Hibernate(); }
void __attribute__((weak)) loop() { }

extern IO::Pin LED;

int main() {
    System::Configure(F_CPU);
    Interrupt::EnableMultiVector();
    Interrupt::InitializeVectorTable();
    JTAG::Disable();


    IdleThread = Thread::Create(IdleThreadFunction, 0, 512);
    MasterThread = Thread::Create(MasterThreadFunction);
    Thread::Start();

    while(1) {
        continue;
    }
    return 0;
}
