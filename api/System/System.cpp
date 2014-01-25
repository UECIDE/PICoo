#include <PICoo.h>

namespace System {


    void __attribute__((nomips16)) Configure(uint32_t clk) {
        uint32_t stInt;
#ifdef _PCACHE
        uint32_t stCache;
        uint32_t wait;
        register uint32_t tmp;
#endif
        stInt = Interrupt::Disable();

        BMXCONCLR = (1 << _BMXCON_BMXWSDRM_POSITION);

#ifdef _PCACHE

        stCache = CHECON;
        stCache |= (3 << _CHECON_PREFEN_POSITION);
        asm("mfc0   %0,$16,0" : "=r"(tmp));
        tmp = (tmp & ~7) | 3;
        asm("mtc0   %0,$16,0" :: "r"(tmp));

        wait = 0;
        while (clk > FLASH_SPEED_HZ) {
            wait += 1;
            clk -= FLASH_SPEED_HZ;
        }
        stCache &= ~_CHECON_PFMWS_MASK;
        stCache |= (wait << _CHECON_PFMWS_POSITION);
        CHECON = stCache;
#endif
        Interrupt::Restore(stInt);
    }
}
