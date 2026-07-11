#include <msp430.h>

/**
 * Exercício 2: Ruído (rebotes) de chaves mecânicas
 * Autor: [Seu Nome]
 * Descrição: O LED vermelho (P1.0) alterna seu estado (liga/desliga)
 * a cada vez que o botão S1 (P2.1) é pressionado.
 * Este código NÃO trata o efeito de "rebote" (bounce).
 */
void main(void) {
    // Para o Watchdog Timer
    WDTCTL = WDTPW | WDTHOLD;

    // --- Configuração dos Pinos (GPIO) ---

    // Configura o pino do LED Vermelho (P1.0) como saída
    P1DIR |= BIT0;
    // Garante que o LED comece apagado
    P1OUT &= ~BIT0;

    // Configura o pino da Chave S1 (P2.1) como entrada com pull-up
    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;

    // --- Laço (Loop) Infinito ---
    while(1) {
        // 1. Espera o botão ser pressionado (nível baixo)
        // O ';' no final cria um laço vazio, uma "trava de execução".
        while((P2IN & BIT1) != 0);

        // 2. Ação: alterna o estado do LED
        P1OUT ^= BIT0;

        // 3. Espera o botão ser solto (nível alto)
        // Essencial para que a ação ocorra apenas uma vez por aperto.
        while((P2IN & BIT1) == 0);
    }
}