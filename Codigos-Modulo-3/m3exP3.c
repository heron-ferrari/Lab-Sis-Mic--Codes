#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TERM_CLR "\033[2J"
#define TERM_HOME "\033[0;0H"

volatile uint8_t nChannels = 1;
volatile uint8_t newConfigFlag = 0;
volatile uint8_t new_nChannels = 1;
volatile uint8_t sampleReady = 0;
volatile uint16_t adcResults[8] = {0};

char buffer[20];
char lcdBuffer[17];

#define SLAVE_ADDR_1 0x27
#define SLAVE_ADDR_2 0x3F

// Store LCD address to avoid repeated scanning
static uint8_t cachedLcdAddr = 0;

void USCI_B0_config(void);
uint8_t i2cSend(uint8_t addr, uint8_t data);
void turnOnBacklight(void);
void clearScreen(void);
uint8_t lcdAddr(void);
void lcdWriteNibble(uint8_t nibble, uint8_t isChar);
void lcdWriteByte(uint8_t byte, uint8_t isChar);
void lcdInit(void);
void lcdWrite(char *str);
void UARTConfig(void);
void uartPrint(char *str);
void ADC_config(void);
void ADC_config_channels(uint8_t channels);
void TimerAConfig(void);
void updateTimerPeriod(void);

//==============================================================================
// I2C CONFIGURATION - BUG FIX: Use |= to preserve UCSWRST
//==============================================================================
void USCI_B0_config(void)
{
    UCB0CTL1 = UCSWRST;                      // Put I2C in reset
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;    // Master, I2C mode, synchronous
    UCB0BRW = 105;                           // Clock divider
    UCB0CTL1 |= UCSSEL_3;                    // FIX: Use |= to keep UCSWRST=1
    P3SEL |= BIT0 | BIT1;                    // P3.0=SDA, P3.1=SCL
    P3REN |= BIT0 | BIT1;                    // Enable pull-ups
    P3OUT |= BIT0 | BIT1;                    // Pull-up (not pull-down)
    UCB0CTL1 &= ~UCSWRST;                    // Release from reset
}

//==============================================================================
// I2C SEND - Unchanged, works correctly
//==============================================================================
uint8_t i2cSend(uint8_t addr, uint8_t data)
{
    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTR | UCTXSTT;
    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0TXBUF = data;
    while (UCB0CTL1 & UCTXSTT)
        ;
    if (UCB0IFG & UCNACKIFG)
    {
        UCB0CTL1 |= UCTXSTP;
        while (UCB0CTL1 & UCTXSTP)
            ;
        return 1;
    }
    while (!(UCB0IFG & UCTXIFG))
        ;
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP)
        ;
    return 0;
}

//==============================================================================
// LCD ADDRESS - Now caches result for efficiency
//==============================================================================
uint8_t lcdAddr(void)
{
    // Return cached address if already found
    if (cachedLcdAddr != 0)
        return cachedLcdAddr;
    
    // Scan for LCD
    if (i2cSend(SLAVE_ADDR_1, 0x00) == 0) {
        cachedLcdAddr = SLAVE_ADDR_1;
        return SLAVE_ADDR_1;
    }
    if (i2cSend(SLAVE_ADDR_2, 0x00) == 0) {
        cachedLcdAddr = SLAVE_ADDR_2;
        return SLAVE_ADDR_2;
    }
    return 0;
}

//==============================================================================
// LCD BACKLIGHT
//==============================================================================
void turnOnBacklight(void)
{
    uint8_t addr = lcdAddr();
    i2cSend(addr, BIT3);        // BIT3 controls backlight
    __delay_cycles(200000);
}

//==============================================================================
// LCD CLEAR SCREEN - BUG FIX: Use proper LCD protocol
//==============================================================================
void clearScreen(void)
{
    lcdWriteByte(0x01, 0);      // FIX: Send clear command through LCD protocol
    __delay_cycles(2000000);    // Wait ~1.53ms for clear to complete
}

//==============================================================================
// LCD BLINK TEST - Unchanged
//==============================================================================
void blink(void)
{
    uint8_t addr = lcdAddr();
    int i;
    for (i = 0; i < 11; i++)
    {
        i2cSend(addr, BIT3);
        __delay_cycles(500000);
        i2cSend(addr, 0x00);
        __delay_cycles(500000);
    }
}

//==============================================================================
// LCD WRITE NIBBLE - Added backlight bit preservation
//==============================================================================
void lcdWriteNibble(uint8_t nibble, uint8_t isChar)
{
    uint8_t addr;
    uint8_t data;

    addr = lcdAddr();
    data = (nibble << 4);       // Data in upper 4 bits

    if (isChar == 1)
        data |= BIT0;           // RS=1 for character
    
    data |= BIT3;               // FIX: Keep backlight ON

    i2cSend(addr, data);        // Setup data with EN=0
    i2cSend(addr, data | BIT2); // Pulse EN=1
    i2cSend(addr, data);        // EN=0 again
}

//==============================================================================
// LCD WRITE BYTE - Unchanged
//==============================================================================
void lcdWriteByte(uint8_t byte, uint8_t isChar)
{
    uint8_t msb, lsb;
    msb = byte / 16;
    lsb = byte % 16;
    lcdWriteNibble(msb, isChar);
    lcdWriteNibble(lsb, isChar);
}

//==============================================================================
// LCD INIT - Unchanged
//==============================================================================
void lcdInit(void)
{
    lcdWriteNibble(0x3, 0);     // Force 8-bit mode
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x3, 0);
    lcdWriteNibble(0x2, 0);     // Switch to 4-bit mode

    lcdWriteByte(0x28, 0);      // 4-bit, 2 lines, 5x8 font
    lcdWriteByte(0x0C, 0);      // Display ON, cursor OFF
    lcdWriteByte(0x06, 0);      // Entry mode: increment
    lcdWriteByte(0x01, 0);      // Clear display
    __delay_cycles(2000000);    // Wait for clear
}

//==============================================================================
// LCD WRITE STRING - Unchanged
//==============================================================================
void lcdWrite(char *str)
{
    uint8_t pos = 0;
    while (*str)
    {
        if (*str == '\n')
        {
            if (pos <= 16)
            {
                lcdWriteByte(0xC0, 0);  // Go to line 2
                str++;
                pos = 16;
            }
            else
            {
                lcdWriteByte(0x80, 0);  // Go back to line 1
                str++;
                pos = 0;
            }
        }
        else
        {
            if (pos == 16)
            {
                lcdWriteByte(0xC0, 0);
            }
            if (pos == 32)
            {
                lcdWriteByte(0x80, 0);
                pos = 0;
            }
            lcdWriteByte(*str++, 1);
            pos++;
        }
    }
}

//==============================================================================
// UART CONFIGURATION - BUG FIX: Correct baud rate for 19200
//==============================================================================
void UARTConfig(void)
{
    UCA1CTL1 = UCSWRST;
    UCA1CTL1 |= UCSSEL_2;       // SMCLK
    
    // FIX: For 19200 bps with 1MHz SMCLK
    // 1000000 / 19200 = 52.08
    UCA1BR0 = 104;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_1;         // Modulation for 19200
    
    P4SEL |= BIT4 | BIT5;       // P4.4=TX, P4.5=RX
    UCA1CTL1 &= ~UCSWRST;
    UCA1IE |= UCRXIE;           // Enable RX interrupt
}

//==============================================================================
// UART PRINT - Unchanged
//==============================================================================
void uartPrint(char *str)
{
    while (*str)
    {
        while (!(UCA1IFG & UCTXIFG))
            ;
        UCA1TXBUF = *str++;
    }
}

//==============================================================================
// ADC CONFIGURATION - Unchanged
//==============================================================================
void ADC_config(void)
{
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1;
    ADC12CTL2 = ADC12RES_2;     // 12-bit resolution
    P6SEL |= 0xFF;              // P6.0-P6.7 as analog inputs
}

//==============================================================================
// ADC CHANNEL CONFIGURATION - Unchanged
//==============================================================================
void ADC_config_channels(uint8_t channels)
{
    uint8_t i;
    ADC12CTL0 &= ~ADC12ENC;
    for (i = 0; i < channels; i++)
    {
        ADC12MCTL[i] = ADC12SREF_0 | i;
    }
    ADC12MCTL[channels - 1] |= ADC12EOS;
    ADC12IE = 0;
    ADC12IE |= (1 << (channels - 1));
    ADC12CTL0 |= ADC12ENC;
}

//==============================================================================
// TIMER CONFIGURATION - Unchanged
//==============================================================================
void TimerAConfig(void)
{
    TA0CCR0 = nChannels * 5000 - 1;
    TA0CTL = TASSEL_2 | MC_1 | TACLR;
    TA0CCTL0 = CCIE;
}

void updateTimerPeriod(void)
{
    TA0CCR0 = nChannels * 5000 - 1;
}

//==============================================================================
// MAIN FUNCTION - BUG FIX: Correct output format per spec
//==============================================================================
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    
    UARTConfig();
    __enable_interrupt();

    USCI_B0_config();
    lcdInit();
    ADC_config();
    ADC_config_channels(nChannels);
    TimerAConfig();
    turnOnBacklight();

    uartPrint(TERM_CLR);
    uartPrint(TERM_HOME);

    while (1)
    {
        // Handle reconfiguration request
        if (newConfigFlag)
        {
            nChannels = new_nChannels;
            ADC12CTL0 &= ~ADC12ENC;
            ADC_config_channels(nChannels);
            updateTimerPeriod();
            newConfigFlag = 0;

            uartPrint(TERM_CLR);
            uartPrint(TERM_HOME);
        }

        // Handle new ADC samples
        if (sampleReady)
        {
            sampleReady = 0;

            // LCD: Show only A0
            sprintf(lcdBuffer, "A1: %03X", adcResults[1]);
            lcdWriteByte(0x80, 0);      // FIX: Move cursor home instead of clear
            lcdWrite(lcdBuffer);

            // UART: Show all channelsb
            // FIX: Removed "Recebendo..." per spec
            // FIX: Use lowercase hex per spec
            uint8_t i;
            for (i = 0; i < nChannels; i++)
            {
                sprintf(buffer, "%d: %03x\r\n", i, adcResults[i]);
                uartPrint(buffer);
            }
        }
    }
}

//==============================================================================
// UART RX INTERRUPT - Unchanged
//==============================================================================
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    char c;
    if (UCA1IFG & UCRXIFG)
    {
        c = UCA1RXBUF;

        if (c >= '1' && c <= '8')
        {
            new_nChannels = c - '0';
            newConfigFlag = 1;
        }
    }
}

//==============================================================================
// TIMER A0 INTERRUPT - Triggers ADC
//==============================================================================
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    ADC12CTL0 |= ADC12SC;
}

//==============================================================================
// ADC INTERRUPT - Stores results
//==============================================================================
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    uint8_t i;
    for (i = 0; i < nChannels; i++)
    {
        adcResults[i] = ADC12MEM[i];
    }
    sampleReady = 1;
}