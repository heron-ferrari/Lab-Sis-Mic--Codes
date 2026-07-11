#include <msp430.h>

/**
 * Exercício 13: Contador binário de 2 bits com interrupções
 * LED verde (P4.7) = LSB | LED vermelho (P1.0) = MSB
 * S1 (P2.1) incrementa | S2 (P1.1) decrementa
 * Debounce por Timer (sem blocking)
 */

volatile unsigned char contador = 0;

// Estados do debounce
volatile unsigned char s1_esperando_debounce = 0;
volatile unsigned char s2_esperando_debounce = 0;
volatile unsigned int timer_debounce = 0;

#define TEMPO_DEBOUNCE 20  // 20ms

void atualizar_leds(void) {
    if (contador & 0x01)
        P4OUT |= BIT7;
    else
        P4OUT &= ~BIT7;
    
    if (contador & 0x02)
        P1OUT |= BIT0;
    else
        P1OUT &= ~BIT0;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    // Configuração dos LEDs
    P1DIR |= BIT0;
    P4DIR |= BIT7;
    P1OUT &= ~BIT0;
    P4OUT &= ~BIT7;

    // Configuração dos botões com pull-up
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;

    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;

    // Timer A1 para debounce (1ms por interrupção)
    TA1CCR0 = 1048;                    // 1.048.576 / 1048 ≈ 1000Hz (1ms)
    TA1CCTL0 = CCIE;                   // Habilita interrupção CCR0
    TA1CTL = TASSEL_2 | MC_1 | TACLR;  // SMCLK, modo UP

    // Configuração das interrupções de GPIO
    P1IES |= BIT1;   // Borda de descida
    P1IFG &= ~BIT1;  // Limpa flag
    P1IE |= BIT1;    // Habilita interrupção

    P2IES |= BIT1;   // Borda de descida
    P2IFG &= ~BIT1;  // Limpa flag
    P2IE |= BIT1;    // Habilita interrupção

    __bis_SR_register(LPM0_bits + GIE);  // Low power + interrupções globais

    while(1) {
        // CPU em sleep
    }
}

// ISR do Timer A1 - executa a cada 1ms
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void) {
    // Gerenciamento do debounce
    if (timer_debounce > 0) {
        timer_debounce--;
        
        if (timer_debounce == 0) {
            // Debounce completo, verificar qual botão
            if (s2_esperando_debounce) {
                // Confirma S2 ainda pressionado
                if (!(P1IN & BIT1)) {
                    // Decrementa
                    if (contador > 0)
                        contador--;
                    else
                        contador = 3;
                    
                    atualizar_leds();
                }
                s2_esperando_debounce = 0;
                P1IE |= BIT1;  // Reabilita interrupção de P1.1
            }
            
            if (s1_esperando_debounce) {
                // Confirma S1 ainda pressionado
                if (!(P2IN & BIT1)) {
                    // Incrementa
                    contador++;
                    if (contador > 3)
                        contador = 0;
                    
                    atualizar_leds();
                }
                s1_esperando_debounce = 0;
                P2IE |= BIT1;  // Reabilita interrupção de P2.1
            }
        }
    }
}

// ISR para S2 (P1.1) - Decrementa
#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
    if (P1IFG & BIT1) {
        // Desabilita interrupção temporariamente
        P1IE &= ~BIT1;
        
        // Inicia debounce
        s2_esperando_debounce = 1;
        timer_debounce = TEMPO_DEBOUNCE;
        
        P1IFG &= ~BIT1;  // Limpa flag
    }
}

// ISR para S1 (P2.1) - Incrementa
#pragma vector=PORT2_VECTOR
__interrupt void Port_2_ISR(void) {
    if (P2IFG & BIT1) {
        // Desabilita interrupção temporariamente
        P2IE &= ~BIT1;
        
        // Inicia debounce
        s1_esperando_debounce = 1;
        timer_debounce = TEMPO_DEBOUNCE;
        
        P2IFG &= ~BIT1;  // Limpa flag
    }
}