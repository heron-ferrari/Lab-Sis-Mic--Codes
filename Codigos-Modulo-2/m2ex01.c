
#include <msp430.h>

/**
 * Exercício 1: Configuração dos pinos
 * Autor: [Seu Nome]
 * Descrição: O LED verde (P4.7) acende quando o botão S1 (P2.1)
 * é pressionado e apaga quando o botão é solto.
 */
void main(void) {
    // Para o Watchdog Timer para evitar que ele reinicie o microcontrolador
    WDTCTL = WDTPW | WDTHOLD;

    // --- Configuração dos Pinos (GPIO) ---

    // Configura o pino do LED Verde (P4.7) como saída
    P1DIR |= BIT0;
    // Garante que o LED comece apagado
    P1OUT &= ~BIT0;

    // Configura o pino da Chave S1 (P2.1) como entrada
    P2DIR &= ~BIT1;
    // Habilita o resistor interno para o pino P2.1
    P2REN |= BIT1;
    // Define o resistor como PULL-UP (nível alto quando solto)
    P2OUT |= BIT1;

    // --- Laço (Loop) Infinito ---
    while(1) {
        // Verifica se o botão S1 está sendo pressionado
        // Com pull-up, o pino vai para nível baixo (0) quando pressionado
        if ((P2IN & BIT1) == 0) {
            // Se for pressionado, acende o LED Verde
            P1OUT |= BIT0;
        } else {
            // Se estiver solto, apaga o LED Verde
            P1OUT &= ~BIT0;
        }
    }
}
