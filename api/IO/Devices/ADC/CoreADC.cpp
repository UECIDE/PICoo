#include <PICoo.h>

namespace IO {

int16_t CoreADC::read(uint16_t pin) {
    int analogValue;
    uint8_t channelNumber;
    uint8_t ain;
    if (pin >= NUM_DIGITAL_PINS_EXTENDED) {
        return 0;
    }
        ain = (pin < NUM_DIGITAL_PINS) ? digitalPinToAnalog(pin) : NOT_ANALOG_PIN;
    if (ain == NOT_ANALOG_PIN) {
        return 0;
    }
    channelNumber = analogInPinToChannel(ain);
#if defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
    p32_ioport *    iop;
    uint16_t        bit;

    /* In MX1/MX2 devices, there is a control register ANSEL associated with
    **  each io port. We need to set the appropriate bit in the ANSEL register
    **  for the io port associated with this pin to ensure that it is in analog
    **  input mode.
    **
    ** Obtain pointer to the registers for this io port.
    */
    iop = portRegisters(digitalPinToPort(pin));

    /* Obtain bit mask for the specific bit for this pin.
    */
    bit = digitalPinToBitMask(pin);

    /* Set the bit in the ANSELx register to ensure that the pin is in
    ** analog input mode.
    */
    iop->ansel.set = bit;
#else
    /* In the other PIC32 devices, all of the analog input capable pins are
    **  in PORTB, and the AD1PCFG register is used to set the pins associated
    **  with PORTB to analog input or digital input mode. Clear the appropriate
    **  bit in AD1PCFG.
    */
    AD1PCFGCLR = (1 << channelNumber);
#endif      // defined(__PIC32MX1XX__) || defined(__PIC32MX2XX__)
    AD1CHS = (channelNumber & 0xFFFF) << 16;
    AD1CON1 =   0; //Ends sampling, and starts converting
    AD1CON1SET = 0b011 << 8; // 16-bit signed fractional value

    /* Set up for manual sampling */
    AD1CSSL =   0;
    AD1CON3 =   0x0002; //Tad = internal 6 Tpb
    AD1CON2 =   0; //analog_reference;

    /* Turn on ADC */
    AD1CON1SET  =   0x8000;

    /* Start sampling */
    AD1CON1SET  =   0x0002;

    /* Delay for a bit */
    Thread::USleep(2);

    /* Start conversion */
    AD1CON1CLR  =   0x0002;

    /* Wait for conversion to finish */
    while (!(AD1CON1 & 0x0001));

    analogValue =   ADC1BUF0;

    return (analogValue);
}


void CoreADC::startSample(uint16_t, int16_t *, uint32_t, uint32_t) {
}

uint8_t CoreADC::getState(uint16_t) {
    return IDLE;
}

void CoreADC::stopSample(uint16_t) {
}


}
