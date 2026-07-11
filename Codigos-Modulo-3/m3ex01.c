#include <msp430.h>
#include <stdint.h>

// Endereços possíveis do PCF8574
#define LCD_ADDR_1 0x3F  // PCF8574AT
#define LCD_ADDR_2 0x27  // PCF8574T

// Comandos para controle da retroiluminação
#define BACKLIGHT_OFF 0x00
#define BACKLIGHT_ON  0x08

void USCI_B0_config(void);
uint8_t i2cSend(uint8_t addr, uint8_t data);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    
    USCI_B0_config();
    
    // Tenta descobrir qual endereço responde
    uint8_t lcd_addr;
    if (i2cSend(LCD_ADDR_1, BACKLIGHT_ON) == 0)
        lcd_addr = LCD_ADDR_1;
    else
        lcd_addr = LCD_ADDR_2;
    
    // Pisca a retroiluminação em 1Hz
    while (1)
    {
        i2cSend(lcd_addr, BACKLIGHT_OFF);
        __delay_cycles(500000);  // 500ms @ 1MHz
        
        i2cSend(lcd_addr, BACKLIGHT_ON);
        __delay_cycles(500000);
    }
}

void USCI_B0_config(void)
{
    UCB0CTL1 = UCSWRST;
    
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    
    UCB0CTL1 |= UCSSEL_3;  // SMCLK
    UCB0BRW = 10;          // Divisor de clock (ajuste conforme necessário)
    
    P3SEL |= BIT0 | BIT1;
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;
    
    UCB0CTL1 &= ~UCSWRST;
}

uint8_t i2cSend(uint8_t addr, uint8_t data)
{
    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTR | UCTXSTT;
    
    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data;
    
    while (UCB0CTL1 & UCTXSTT);
    
    if (UCB0IFG & UCNACKIFG)
    {
        UCB0CTL1 |= UCTXSTP;
        while (UCB0CTL1 & UCTXSTP);
        UCB0IFG &= ~UCNACKIFG;
        return 1;
    }
    
    while (!(UCB0IFG & UCTXIFG));
    
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    
    return 0;
}