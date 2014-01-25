#include <PICoo.h>

extern const uint32_t __attribute__((section(".mpide_version"))) _verMPIDE_Stub = MPIDEVER; 

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

int main() {
    System::Configure(F_CPU);
    Interrupt::EnableMultiVector();
    Interrupt::InitializeVectorTable();
    JTAG::Disable();

    IdleThread = Thread::Create(IdleThreadFunction, 80);
    MasterThread = Thread::Create(MasterThreadFunction);
    Thread::Start();

    while(1) {
        continue;
    }
    return 0;
}
