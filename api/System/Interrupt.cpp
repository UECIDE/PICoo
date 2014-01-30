#include <PICoo.h>

void (*volatile _isr_primary_install[NUM_INT_VECTOR])(void);

namespace Interrupt {

    isrFunc _isr_table[NUM_INT_REQUEST];

    void __attribute__((nomips16)) Restore(uint32_t status) {
        if (status & 0x00000001) {
            asm volatile("ei");
        } else {
            asm volatile("di");
        }
    }

    uint32_t __attribute__((nomips16)) Enable() {
        uint32_t status = 0;
        asm volatile("ei    %0" : "=r"(status));
        return status;
    }

    uint32_t __attribute__((nomips16)) Disable() {
        uint32_t status = 0;
        asm volatile("di    %0" : "=r"(status));
        return status;
    }

    void __attribute__((nomips16)) EnableMultiVector() {
        uint32_t val;
        asm volatile("mfc0  %0,$13" : "=r"(val));
        val |= 0x00800000;
        asm volatile("mtc0  %0,$13" : "+r"(val));

        INTCONSET = _INTCON_MVEC_MASK;
        Enable();
    }

    void __attribute__((nomips16)) EnableSingleVector() {
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
        Enable();
    }


    void InitializeVectorTable() {
        int i = 0;
        void *orgIntVec = (void *)_image_header_info.pOrgVector0;

        for (i = 0; i < NUM_INT_VECTOR; i++) {
            if (*((uint32_t *)orgIntVec) != 0xFFFFFFFF) {
                _isr_primary_install[i] = (isrFunc) orgIntVec;
            } else {
                _isr_primary_install[i] = (isrFunc) &_GEN_EXCPT_ADDR;
            }
            orgIntVec += _image_header_info.cbVectorSpacing;
        }
        for (i = 0; i < NUM_INT_REQUEST; i++) {
            _isr_table[i] = NULL;
        }
    }

    void SetPriority(uint32_t vec, uint32_t ipl, uint32_t spl) {
        p32_regset *ipc;
        uint32_t bn;

        ipc = ((p32_regset *)&IPC0) + (vec >>2);
        bn = Math::MulU(8, (vec & 0x03 ));
        ipc->clr = (0x1F << bn);
        ipc->set = ((ipl << 2) + spl) << bn;
    }

    isrFunc SetVector(uint32_t vec, isrFunc func) {
        isrFunc t = 0;
        if (vec < NUM_INT_VECTOR) {
            t = _isr_primary_install[vec];
            _isr_primary_install[vec] = func;
        }
        return t;
    }

    isrFunc GetVector(uint32_t vec) {
        if (vec < NUM_INT_VECTOR) {
            return _isr_primary_install[vec];
        }
        return 0;
    }

    isrFunc ClearVector(uint32_t vec) {
        if (vec < NUM_INT_VECTOR) {
            isrFunc f = _isr_primary_install[vec];
            SetPriority(vec, 0, 0);
            _isr_primary_install[vec] = (isrFunc) &_GEN_EXCPT_ADDR;
            return f;
        }
        return 0;
    }

    uint32_t GetFlag(uint32_t irq) {
        p32_regset *ifs = ((p32_regset *)&IFS0) + (irq >> 5);
        return (ifs->reg & (1 << (irq & 31))) != 0;
    }
            
    void ClearFlag(uint32_t irq) {
        p32_regset *ifs = ((p32_regset *)&IFS0) + (irq >> 5);
        ifs->clr = (1 << (irq & 31));
    }

    uint32_t EnableIRQ(uint32_t irq) {
        p32_regset *    iec;
        uint32_t        st;

        iec = ((p32_regset *)&IEC0) + (irq >> 5);
        st = iec->reg;
        iec->set = 1 << (irq & 31);
        return st;
    }

    uint32_t DisableIRQ(uint32_t irq) {
        p32_regset *iec = ((p32_regset *)&IEC0) + (irq >> 5);
        uint32_t st = iec->reg;
        iec->clr = 1 << (irq & 31);
        return st;
    }

    uint8_t isEnabled(uint32_t irq) {
        p32_regset *iec = ((p32_regset *)&IEC0) + (irq >> 5);
        return (iec->reg & (1<<(irq & 31))) ? true : false;
    }

    void ExecuteInterrupt(uint32_t i) {
        if (i < NUM_INT_REQUEST) {
            if (_isr_table[i] != NULL) {
                _isr_table[i]();
            }
        }
    }

    void AttachInterrupt(uint32_t i, isrFunc f) {
        if (i < NUM_INT_REQUEST) {
            _isr_table[i] = f;
        }
    }

    void DetatchInterrupt(uint32_t i) {
        if (i < NUM_INT_REQUEST) {
            _isr_table[i] = NULL;
        }
    }
}
