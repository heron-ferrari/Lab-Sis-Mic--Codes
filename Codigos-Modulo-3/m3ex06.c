#include <msp430.h>
#include <stdint.h>

// Endereços possíveis do PCF8574
#define LCD_ADDR_1 0x3F  // PCF8574AT
#define LCD_ADDR_2 0x27  // PCF8574T

// Bits do PCF8574 conectados ao LCD
#define RS    (1<<0)  // Register Select
#define RW    (1<<1)  // Read/Write
#define EN    (1<<2)  // Enable
#define BL    (1<<3)  // Backlight
#define D4    (1<<4)  // Data bit 4
#define D5    (1<<5)  // Data bit 5
#define D6    (1<<6)  // Data bit 6
#define D7    (1<<7)  // Data bit 7

// Comandos LCD
#define LCD_CLEAR           0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE      0x06
#define LCD_DISPLAY_ON      0x0C
#define LCD_DISPLAY_OFF     0x08
#define LCD_CURSOR_ON       0x0E
#define LCD_BLINK_ON        0x0F
#define LCD_4BIT_2LINE      0x28
#define LCD_8BIT_1LINE      0x30

// Variável global para controlar backlight
uint8_t backlight = BL;
uint8_t lcd_addr;

// Protótipos
void USCI_B0_config(void);
uint8_t i2cSend(uint8_t addr, uint8_t data);
void lcdWriteNibble(uint8_t nibble, uint8_t isChar);
void lcdWriteByte(uint8_t byte, uint8_t isChar);
void lcdInit(void);
void lcdWrite(char *str);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    
    USCI_B0_config();
    
    // Descobre qual endereço responde
    if (i2cSend(LCD_ADDR_1, BL) == 0)
        lcd_addr = LCD_ADDR_1;
    else
        lcd_addr = LCD_ADDR_2;
    
    lcdInit();
    
    // Escreve uma string no LCD
    lcdWrite("Heron e Gabriel");
    
    while(1);
}

void USCI_B0_config(void)
{
    UCB0CTL1 = UCSWRST;
    
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0CTL1 |= UCSSEL_2;  // Usa SMCLK
    UCB0BRW = 10;          // Divide para obter ~100kHz
    
    P3SEL |= BIT0 | BIT1;
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;
    P3DIR &= ~(BIT0 | BIT1);
    
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

// Exercício 3: Escreve um nibble no LCD
void lcdWriteNibble(uint8_t nibble, uint8_t isChar)
{
    uint8_t data;
    
    // Prepara o byte com RS, dados e backlight, EN=0
    data = backlight;
    if (isChar) data |= RS;
    data |= ((nibble & 0x0F) << 4);  // Coloca nibble nos bits D7-D4
    i2cSend(lcd_addr, data);
    __delay_cycles(100);
    
    // Pulso de EN (subida)
    data |= EN;
    i2cSend(lcd_addr, data);
    __delay_cycles(100);
    
    // Pulso de EN (descida)
    data &= ~EN;
    i2cSend(lcd_addr, data);
    __delay_cycles(100);
}

// Exercício 4: Escreve um byte no LCD (dois nibbles)
void lcdWriteByte(uint8_t byte, uint8_t isChar)
{
    lcdWriteNibble(byte >> 4, isChar);    // Envia MSB primeiro
    lcdWriteNibble(byte & 0x0F, isChar);  // Depois LSB
    __delay_cycles(2000);
}

// Exercício 5: Inicializa o LCD
void lcdInit(void)
{
    __delay_cycles(50000);  // Espera > 40ms após power-on
    
    // Sequência de inicialização: envia 0x3 três vezes
    lcdWriteNibble(0x3, 0);
    __delay_cycles(5000);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(200);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(200);
    
    // Agora coloca em modo 4 bits
    lcdWriteNibble(0x2, 0);
    __delay_cycles(200);
    
    // Configurações (agora usa lcdWriteByte)
    lcdWriteByte(LCD_4BIT_2LINE, 0);  // 4 bits, 2 linhas, fonte 5x8
    lcdWriteByte(LCD_DISPLAY_OFF, 0); // Display off
    lcdWriteByte(LCD_CLEAR, 0);       // Limpa display
    __delay_cycles(2000);             // Espera 1.53ms
    lcdWriteByte(LCD_ENTRY_MODE, 0);  // Incrementa cursor
    lcdWriteByte(LCD_DISPLAY_ON, 0);  // Display on, cursor off
}

// Exercício 6: Escreve uma string no LCD
void lcdWrite(char *str)
{
    uint8_t line = 0;
    uint8_t col = 0;
    
    while (*str != '\0')
    {
        // Verifica quebra de linha
        if (col >= 16)
        {
            col = 0;
            line++;
            
            if (line >= 2)
                break;  // LCD tem apenas 2 linhas
            
            // Move cursor para segunda linha (endereço 0x40)
            lcdWriteByte(0x80 | 0x40, 0);
        }
        
        // Escreve o caractere (isChar = 1)
        lcdWriteByte(*str, 1);
        str++;
        col++;
    }
}