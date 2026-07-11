#include <msp430.h>

#define _1_x_1 0xBA45
#define _1_x_2 0xB946
#define _1_x_3 0xB847
#define _2_x_1 0xBB44
#define _2_x_2 0xBF40
#define _2_x_3 0xBC43
#define _3_x_1 0xF807
#define _3_x_2 0xEA15
#define _3_x_3 0xF609

#define POT100 10000
#define START_MAX 14500
#define START_MIN 13500
#define BIT1_MAX 2450
#define BIT1_MIN 2250
#define BIT0_MAX 1340
#define BIT0_MIN 1120
#define PWM_STEP 2000  // 20% de 10000

volatile int i = 0;
volatile int period = 0;
volatile int vector[32];
volatile unsigned long code;
volatile unsigned long key;
volatile int green_led_pwm = POT100;  // Inicia em 100%
volatile int green_led_state = 1;     // 1 = aceso, 0 = apagado

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    config_leds();
    config_tb0_1();
    config_ta1_1();

    while (1)
    {
        config_falling_ta1_1();
        wait_first_falling_edge();
        wait_second_falling_edge();

        if ((period > START_MIN) && (period < START_MAX))
        {
            config_rising_ta1_1();
            wait_rising_edge();
            write_vector();
        }

        vector_into_words();
        execute_instructions(key);
        _delay_cycles(100);
    }
}

void write_vector(void)
{
    for (i = 0; i < 32; i++)
    {
        while ((TA1CCTL1 & CCIFG) == 0)
            ;
        TA1CTL |= TACLR;
        TA1CCTL1 &= ~CCIFG;
        vector[i] = TA1CCR1;
    }
}

void vector_into_words(void)
{
    code = 0;  // Limpa o código anterior
    for (i = 0; i < 32; i++)
    {
        code = code >> 1;
        if ((vector[i] > BIT1_MIN) && (vector[i] < BIT1_MAX))
        {
            code |= 0x80000000L;
        }
    }
    key = code >> 16;
}

void config_ta1_1(void)
{
    TA1CTL = TASSEL_2 | MC_2;
    P2DIR &= ~BIT0;
    P2REN |= BIT0;
    P2OUT |= BIT0;
    P2SEL |= BIT0;
    TA1CCTL1 &= ~CCIFG;
}

void config_rising_ta1_1(void)
{
    TA1CCTL1 = CM_1 | CCIS_0 | SCS | CAP;
    TA1CCTL1 &= ~CCIFG;
}

void config_falling_ta1_1(void)
{
    TA1CCTL1 = CM_2 | CCIS_0 | SCS | CAP;
    TA1CCTL1 &= ~CCIFG;
}

void wait_first_falling_edge(void)
{
    while ((TA1CCTL1 & CCIFG) == 0)
        ;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
}

void wait_second_falling_edge(void)
{
    while ((TA1CCTL1 & CCIFG) == 0)
        ;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
    period = TA1CCR1;
}

void wait_rising_edge(void)
{
    while ((TA1CCTL1 & CCIFG) == 0)
        ;
    TA1CTL |= TACLR;
    TA1CCTL1 &= ~CCIFG;
}

void config_leds(void)
{
    // LED Vermelho P1.0 - saída digital simples
    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // LED Verde P4.7 - PWM
    P4OUT &= ~BIT7;
    P4DIR |= BIT7;
    P4SEL |= BIT7;
    PMAPKEYID = 0x02D52;
    P4MAP7 = PM_TB0CCR1A;
}

void decrease_green_pwm(void)
{
    green_led_pwm -= PWM_STEP;
    if (green_led_pwm < 0)
        green_led_pwm = 0;
    
    if (green_led_state == 1)
        TB0CCR1 = green_led_pwm;
}

void increase_green_pwm(void)
{
    green_led_pwm += PWM_STEP;
    if (green_led_pwm > POT100)
        green_led_pwm = POT100;
    
    if (green_led_state == 1)
        TB0CCR1 = green_led_pwm;
}

void toggle_green_pwm(void)
{
    green_led_state = !green_led_state;
    
    if (green_led_state == 1)
        TB0CCR1 = green_led_pwm;  // Acende com intensidade atual
    else
        TB0CCR1 = 0;  // Apaga
}

void execute_instructions(int key)
{
    switch (key)
    {
    // GRUPO 1 - LED Vermelho ON/OFF
    case _1_x_1:
        P1OUT |= BIT0;  // Liga LED vermelho
        break;
    case _1_x_2:
        P1OUT &= ~BIT0;  // Desliga LED vermelho
        break;
    case _1_x_3:
        P1OUT ^= BIT0;  // Toggle LED vermelho
        break;
    
    // GRUPO 2 - LED Verde ON/OFF
    case _2_x_1:
        TB0CCR1 = POT100;  // Liga LED verde 100%
        green_led_pwm = POT100;
        green_led_state = 1;
        break;
    case _2_x_2:
        TB0CCR1 = 0;  // Desliga LED verde
        green_led_state = 0;
        break;
    case _2_x_3:
        toggle_green_pwm();  // Toggle LED verde
        break;
    
    // GRUPO 3 - LED Verde PWM (intensidade)
    case _3_x_1:
        decrease_green_pwm();  // Diminui 20%
        break;
    case _3_x_2:
        toggle_green_pwm();  // Toggle
        break;
    case _3_x_3:
        increase_green_pwm();  // Aumenta 20%
        break;
    
    default:
        break;
    }
}

void config_tb0_1(void)
{
    TB0CTL = TBSSEL_2 | MC_1;
    TB0CCR0 = POT100;
    TB0CCTL1 = OUTMOD_7;  // Reset/Set mode
    TB0CCR1 = POT100;
}