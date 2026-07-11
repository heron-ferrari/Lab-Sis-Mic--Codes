#include <msp430.h>

/**
 * Exercício 10: PWM Variável
 * LED vermelho controlado por PWM (128Hz) via Timer A0 canal 1
 * S2 (P1.1) aumenta duty cycle | S1 (P2.1) diminui duty cycle
 * Passos de 12.5% (1/8 do período)
 */

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    // Configuração do PWM no pino P1.2
    P1DIR |= BIT2;
    P1SEL |= BIT2;

    // Configuração dos botões com pull-up
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;

    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;

    // Timer A0: 128Hz PWM
    TA0CCR0 = 8191;              // Período (1.048.576 / 8192 = 128Hz)
    TA0CCR1 = 4096;              // Duty cycle inicial 50%
    TA0CCTL1 = OUTMOD_7;         // PWM mode
    TA0CTL = TASSEL_2 | MC_1 | TACLR;

    const int step = 1024;       // 12.5% do período

    unsigned char s1_ant = 1;
    unsigned char s2_ant = 1;

    while(1) {
        // Botão S2: aumenta duty cycle
        unsigned char s2_atual = (P1IN & BIT1) ? 1 : 0;
        if (s2_ant && !s2_atual) {
            __delay_cycles(100000);
            if (!(P1IN & BIT1)) {
                if (TA0CCR1 <= TA0CCR0 - step)
                    TA0CCR1 += step;
                else
                    TA0CCR1 = TA0CCR0;
            }
        }
        s2_ant = s2_atual;

        // Botão S1: diminui duty cycle
        unsigned char s1_atual = (P2IN & BIT1) ? 1 : 0;
        if (s1_ant && !s1_atual) {
            __delay_cycles(100000);
            if (!(P2IN & BIT1)) {
                if (TA0CCR1 >= step)
                    TA0CCR1 -= step;
                else
                    TA0CCR1 = 0;
            }
        }
        s1_ant = s1_atual;
    }
}