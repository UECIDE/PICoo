#include <PICoo.h>

isrFunc _isr_table[NUM_INT_REQUEST];
thread ISRThread;

IO::Pin *_indicator;


extern "C" {
    extern void ISRWrapper();
    void __attribute__((interrupt)) processFastISR();
}

void (*volatile _isr_primary_install[NUM_INT_VECTOR])(void);

void isrThreadStub(uint32_t x) {
    while(1) {
        Thread::hibernate();
    }
}

void __attribute__((nomips16)) Interrupt::restore(uint32_t status) {
    if (status & 0x00000001) {
        asm volatile("ei");
    } else {
        asm volatile("di");
    }
}

uint32_t __attribute__((nomips16)) Interrupt::enable() {
    uint32_t status = 0;
    asm volatile("ei    %0" : "=r"(status));
    return status;
}

uint32_t __attribute__((nomips16)) Interrupt::disable() {
    uint32_t status = 0;
    asm volatile("di    %0" : "=r"(status));
    return status;
}

void __attribute__((nomips16)) Interrupt::enableMultiVector() {
    uint32_t val;
    asm volatile("mfc0  %0,$13" : "=r"(val));
    val |= 0x00800000;
    asm volatile("mtc0  %0,$13" : "+r"(val));

    INTCONSET = _INTCON_MVEC_MASK;
    enable();

    ISRThread = Thread::create("[kernel]", isrThreadStub);
    setPriority(0, 1, 0);
    setVector(0, ISRWrapper);
    _indicator = NULL;
}

void __attribute__((nomips16)) Interrupt::enableSingleVector() {
    uint32_t val;
    asm volatile("mfc0  %0,$12" : "=r"(val));
    val |= 0x00400000;
    asm volatile("mtc0  %0,$12" : "+r"(val));

    asm volatile("mfc0  %0,$13" : "=r"(val));
    val |= 0x00800000;
    asm volatile("mtc0  %0,$13" : "+r"(val));

    asm volatile("mfc0  %0,$12" : "=r"(val));
    val &= 0xFFBFFFFD;
    asm volatile("mtc0  %0,$12" : "+r"(val));

    INTCONCLR = _INTCON_MVEC_MASK;
    enable();

    ISRThread = Thread::create("[kernel_isr]", isrThreadStub);
    setPriority(0, 1, 0);
    setVector(0, ISRWrapper);
    _indicator = NULL;
}

void Interrupt::initializeVectorTable() {
    int i = 0;
    void *orgIntVec = (void *)_image_header_info.pOrgVector0;

    for (i = 0; i < NUM_INT_VECTOR; i++) {
        _isr_primary_install[i] = (isrFunc) &processFastISR;
        setPriority(i, 4, 0);
        orgIntVec += _image_header_info.cbVectorSpacing;
    }
    for (i = 0; i < NUM_INT_REQUEST; i++) {
        _isr_table[i] = NULL;
    }
}

void Interrupt::setPriority(uint32_t vec, uint32_t ipl, uint32_t spl) {
    p32_regset *ipc;
    uint32_t bn;

    ipc = ((p32_regset *)&IPC0) + (vec >>2);
    bn = Math::mulU(8, (vec & 0x03 ));
    ipc->clr = (0x1F << bn);
    ipc->set = ((ipl << 2) + spl) << bn;
}

isrFunc Interrupt::setVector(uint32_t vec, isrFunc func) {
    isrFunc t = 0;
    if (vec < NUM_INT_VECTOR) {
        t = _isr_primary_install[vec];
        _isr_primary_install[vec] = func;
    }
    return t;
}

isrFunc Interrupt::getVector(uint32_t vec) {
    if (vec < NUM_INT_VECTOR) {
        return _isr_primary_install[vec];
    }
    return 0;
}

isrFunc Interrupt::clearVector(uint32_t vec) {
    if (vec < NUM_INT_VECTOR) {
        isrFunc f = _isr_primary_install[vec];
        setPriority(vec, 0, 0);
        _isr_primary_install[vec] = (isrFunc) &processFastISR; //&_GEN_EXCPT_ADDR;
        return f;
    }
    return 0;
}

uint32_t Interrupt::getFlag(uint32_t irq) {
    p32_regset *ifs = ((p32_regset *)&IFS0) + (irq >> 5);
    return (ifs->reg & (1 << (irq & 31))) != 0;
}
        
void Interrupt::clearFlag(uint32_t irq) {
    p32_regset *ifs = ((p32_regset *)&IFS0) + (irq >> 5);
    ifs->clr = (1 << (irq & 31));
}

uint32_t Interrupt::enableIRQ(uint32_t irq) {
    p32_regset *    iec;
    uint32_t        st;

    iec = ((p32_regset *)&IEC0) + (irq >> 5);
    st = iec->reg;
    iec->set = 1 << (irq & 31);
    return st;
}

uint32_t Interrupt::disableIRQ(uint32_t irq) {
    p32_regset *iec = ((p32_regset *)&IEC0) + (irq >> 5);
    uint32_t st = iec->reg;
    iec->clr = 1 << (irq & 31);
    return st;
}

uint8_t Interrupt::isEnabled(uint32_t irq) {
    p32_regset *iec = ((p32_regset *)&IEC0) + (irq >> 5);
    return (iec->reg & (1<<(irq & 31))) ? true : false;
}

void Interrupt::executeInterrupt(uint32_t i) {
    if (i < NUM_INT_REQUEST) {
        if (_isr_table[i] != NULL) {
            _isr_table[i]();
        }
    }
}

void Interrupt::attachInterrupt(uint32_t i, isrFunc f) {
    if (i < NUM_INT_REQUEST) {
        _isr_table[i] = f;
    }
}

void Interrupt::detatchInterrupt(uint32_t i) {
    if (i < NUM_INT_REQUEST) {
        _isr_table[i] = NULL;
    }
}

extern "C" {
    void processInterrupt() {
        Interrupt::indicateOn();
        for (uint32_t irq = 0; irq < NUM_INT_REQUEST; irq++) {
            if (Interrupt::getFlag(irq)) {
                Interrupt::executeInterrupt(irq);
                Interrupt::clearFlag(irq);
                break;
            }
        }
        Interrupt::indicateOff();
    }
    void __attribute__((interrupt)) processFastISR() {
        for (uint32_t irq = 1; irq < NUM_INT_REQUEST; irq++) {
            if (Interrupt::getFlag(irq)) {
                Interrupt::executeInterrupt(irq);
                Interrupt::clearFlag(irq);
                break;
            }
        }
    }
}

void Interrupt::setIndicatorPin(IO::Pin& p) {
    _indicator = &p;
}

void Interrupt::indicateOn() {
    if (_indicator != NULL) {
        _indicator->write(IO::HIGH);
    }
}

void Interrupt::indicateOff() {
    if (_indicator != NULL) {
        _indicator->write(IO::LOW);
    }
}
