#include <msp430.h>

// --- CONFIGURAÇÃO DOS PINOS DO LCD (Porta 4) ---
#define I2C_PORT_DIR  P4DIR
#define I2C_PORT_OUT  P4OUT
#define I2C_PORT_IN   P4IN
#define SDA_PIN       BIT1  // P4.1 (Dados)
#define SCL_PIN       BIT2  // P4.2 (Clock)

// --- CONFIGURAÇÃO LCD ---
#define LCD_ADDR      0x27 
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
#define LCD_COMMAND   0x00
#define LCD_DATA      0x01

// --- RELÉ E BOMBA ---
#define RELAY_PIN     BIT0 // P2.0
#define RELAY_PORT_DIR P2DIR
#define RELAY_PORT_OUT P2OUT

// --- CALIBRAÇÃO ---
#define LIMIAR_SECO  3950 
#define LIMIAR_UMIDO 3600 

// --- ESTADOS ---
#define ESTADO_OK     0
#define ESTADO_REGANDO 1

// --- PROTÓTIPOS ---
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);

// Funções Software I2C
void soft_i2c_init();
void soft_i2c_start();
void soft_i2c_stop();
void soft_i2c_write_byte(unsigned char data);
void soft_i2c_pulse();

// Funções LCD
void lcd_send_nibble(unsigned char val, unsigned char mode);
void lcd_write_byte(unsigned char val, unsigned char mode);
void lcd_init();
void lcd_print(char *str);
void lcd_print_number(unsigned int num);

// Hardware
void adc_init();
unsigned int adc_read();
void relay_init();
void relay_set(unsigned char state);

// Variáveis Globais
unsigned char estado_atual = ESTADO_OK;
unsigned char estado_anterior = 255; 

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;   

    relay_init(); 
    delay_ms(500); 

    soft_i2c_init(); // P4.1 e P4.2
    lcd_init();
    adc_init();      // P6.0

    lcd_print("BABA ELETRONICA");
    lcd_write_byte(0xC0, LCD_COMMAND);
    lcd_print("Iniciando...");
    delay_ms(2000);
    lcd_write_byte(0x01, LCD_COMMAND); 

    unsigned int leitura = 0;

    while(1) {
        leitura = adc_read();

        // 1. Definição do Estado
        if (leitura > LIMIAR_SECO) {
            estado_atual = ESTADO_REGANDO;
        } else if (leitura < LIMIAR_UMIDO) {
            estado_atual = ESTADO_OK;
        }

        // 2. Gerenciamento de Mudança de Estado (Texto e Bomba)
        if (estado_atual != estado_anterior) {
            
            if (estado_atual == ESTADO_REGANDO) {
                relay_set(1); // Liga bomba
            } else {
                relay_set(0); // Desliga bomba
            }

            delay_ms(200); // Pausa para estabilizar tensão

            // Limpa e atualiza os textos fixos
            lcd_write_byte(0x01, LCD_COMMAND); 
            delay_ms(2);
            
            if (estado_atual == ESTADO_REGANDO) {
                lcd_print("SOLO: SECO");
                lcd_write_byte(0xC0, LCD_COMMAND);
                lcd_print("STATUS: REGANDO");
            } else {
                lcd_print("SOLO: UMIDO");
                lcd_write_byte(0xC0, LCD_COMMAND);
                lcd_print("STATUS: OK");
            }
            
            estado_anterior = estado_atual;
        }

        // 3. ATUALIZAÇÃO DO NÚMERO
        // Move o cursor para o final da primeira linha (Posição 12 - 0x8C)
        lcd_write_byte(0x8C, LCD_COMMAND); 
        lcd_print_number(leitura);
        lcd_print("  "); // Espaços para limpar dígitos antigos (ex: 4000 -> 900)

        delay_ms(500); 
    }
}

// --- DRIVER I2C SOFTWARE (PORTA 4) ---
void soft_i2c_init() {
    I2C_PORT_DIR |= SDA_PIN + SCL_PIN;
    I2C_PORT_OUT |= SDA_PIN + SCL_PIN;
}

void soft_i2c_start() {
    I2C_PORT_OUT |= SDA_PIN;
    I2C_PORT_OUT |= SCL_PIN;
    delay_us(5);
    I2C_PORT_OUT &= ~SDA_PIN;
    delay_us(5);
    I2C_PORT_OUT &= ~SCL_PIN;
}

void soft_i2c_stop() {
    I2C_PORT_OUT &= ~SDA_PIN;
    I2C_PORT_OUT |= SCL_PIN;
    delay_us(5);
    I2C_PORT_OUT |= SDA_PIN;
    delay_us(5);
}

void soft_i2c_pulse() {
    I2C_PORT_OUT |= SCL_PIN;
    delay_us(5);
    I2C_PORT_OUT &= ~SCL_PIN;
    delay_us(5);
}

void soft_i2c_write_byte(unsigned char data) {
    unsigned char i;
    for(i = 0; i < 8; i++) {
        if (data & 0x80) I2C_PORT_OUT |= SDA_PIN;
        else I2C_PORT_OUT &= ~SDA_PIN;
        soft_i2c_pulse();
        data <<= 1;
    }
    // ACK Dummy
    I2C_PORT_DIR &= ~SDA_PIN; 
    I2C_PORT_OUT |= SCL_PIN;  
    delay_us(5);
    I2C_PORT_OUT &= ~SCL_PIN; 
    I2C_PORT_DIR |= SDA_PIN;  
    delay_us(5);
}

// --- RELÉ ---
void relay_init() {
    RELAY_PORT_DIR |= RELAY_PIN;
    RELAY_PORT_OUT |= RELAY_PIN; 
}

void relay_set(unsigned char state) {
    if (state) RELAY_PORT_OUT &= ~RELAY_PIN; 
    else RELAY_PORT_OUT |= RELAY_PIN;        
}

// --- ADC (P6.0) ---
void adc_init() {
    P6SEL |= BIT0; 
    ADC12CTL0 = ADC12SHT0_2 + ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL2 = ADC12RES_2; 
    ADC12MCTL0 = ADC12INCH_0; 
}

unsigned int adc_read() {
    ADC12CTL0 |= ADC12ENC + ADC12SC;
    while (ADC12CTL0 & ADC12BUSY);
    return ADC12MEM0;
}

// --- UTILITÁRIOS ---
void delay_ms(unsigned int ms) {
    while (ms--) __delay_cycles(1000); 
}

void delay_us(unsigned int us) {
    while (us--) __delay_cycles(1);
}

// --- LCD ---
void lcd_send_nibble(unsigned char val, unsigned char mode) {
    unsigned char data = val & 0xF0;
    data |= mode | LCD_BACKLIGHT;
    soft_i2c_start();
    soft_i2c_write_byte(LCD_ADDR << 1);
    soft_i2c_write_byte(data);
    soft_i2c_write_byte(data | LCD_ENABLE);
    soft_i2c_write_byte(data);
    soft_i2c_stop();
}

void lcd_write_byte(unsigned char val, unsigned char mode) {
    unsigned char high_nibble = val & 0xF0;
    unsigned char low_nibble = (val << 4) & 0xF0;
    lcd_send_nibble(high_nibble, mode);
    lcd_send_nibble(low_nibble, mode);
}

void lcd_init() {
    delay_ms(50);
    lcd_send_nibble(0x30, LCD_COMMAND);
    delay_ms(5);
    lcd_send_nibble(0x30, LCD_COMMAND);
    delay_ms(1);
    lcd_send_nibble(0x30, LCD_COMMAND);
    lcd_send_nibble(0x20, LCD_COMMAND);
    lcd_write_byte(0x28, LCD_COMMAND);
    lcd_write_byte(0x0C, LCD_COMMAND);
    lcd_write_byte(0x06, LCD_COMMAND);
    lcd_write_byte(0x01, LCD_COMMAND);
    delay_ms(5);
}

void lcd_print(char *str) {
    while (*str) lcd_write_byte(*str++, LCD_DATA);
}

void lcd_print_number(unsigned int num) {
    char str[6];
    int i = 0;
    if (num == 0) { lcd_print("0"); return; }
    unsigned int temp = num;
    while (temp > 0) { str[i++] = (temp % 10) + '0'; temp /= 10; }
    while (i > 0) { lcd_write_byte(str[--i], LCD_DATA); }
}