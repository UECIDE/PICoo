#include <PICoo.h>

namespace JTAG {
    void Enable() {
#if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
        CFGCONbits.JTAGEN = 1;
        //CFGCONbits.TDOEN = 0;
        //OSCCONbits.SOSCEN = 0;
#else
        DDPCONbits.JTAGEN   =   1;
#endif
    }

    void Disable() {
#if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
        CFGCONbits.JTAGEN = 0;
        //CFGCONbits.TDOEN = 0;
        //OSCCONbits.SOSCEN = 0;
#else
        DDPCONbits.JTAGEN   =   0;
#endif
    }
}

