#include <PICoo.h>

namespace JTAG {
    void enable() {
#if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
        CFGCONbits.JTAGEN = 1;
        //CFGCONbits.TDOEN = 0;
        //OSCCONbits.SOSCEN = 0;
#else
        DDPCONbits.JTAGEN   =   1;
#endif
    }

    void disable() {
#if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
        CFGCONbits.JTAGEN = 0;
        //CFGCONbits.TDOEN = 0;
        //OSCCONbits.SOSCEN = 0;
#else
        DDPCONbits.JTAGEN   =   0;
#endif
    }
}

