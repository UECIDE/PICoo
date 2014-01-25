#include <PICoo.h>

void (*volatile _isr_primary_install[NUM_INT_VECTOR])(void);

namespace Interrupt {
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
    }

    void SetPriority(uint32_t vec, uint32_t ipl, uint32_t spl) {
        p32_regset *ipc;
        uint32_t bn;

        ipc = ((p32_regset *)&IPC0) + (vec / 4);
        bn = 8 * (vec % 4);
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
        p32_regset *ifs = ((p32_regset *)&IFS0) + (irq / 32);
        return (ifs->reg & (1 << (irq % 32))) != 0;
    }
            
    void ClearFlag(uint32_t irq) {
        p32_regset *ifs = ((p32_regset *)&IFS0) + (irq / 32);
        ifs->clr = (1 << (irq % 32));
    }

    uint32_t EnableIRQ(uint32_t irq) {
        p32_regset *    iec;
        uint32_t        st;

        iec = ((p32_regset *)&IEC0) + (irq / 32);
        st = iec->reg;
        iec->set = 1 << (irq % 32);
        return st;
    }

    uint32_t DisableIRQ(int irq) {
        p32_regset *    iec;
        uint32_t        st;

        iec = ((p32_regset *)&IEC0) + (irq / 32);
        st = iec->reg;
        iec->clr = 1 << (irq % 32);
        return st;
    }

            
}
