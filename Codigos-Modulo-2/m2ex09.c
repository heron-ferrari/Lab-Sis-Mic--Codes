#include <msp430.h>

/**
 * Exercício 9: PWM por Hardware
 * Descrição: Gera uma onda PWM de 128 Hz diretamente na saída de hardware
 * do Timer_A0 (TA0.1 no pino P1.2) para controlar o LED vermelho.
 */
 
void main(void) {
    // Parar o Watchdog Timer
    WDTCTL = WDTPW | WDTHOLD;

    // Configuração do GPIO
    P1DIR |= BIT2;          // Configura P1.2 como saída
    P1SEL |= BIT2;          // Seleciona a função alternativa para P1.2 (TA0.1)

    // Configuração do Timer_A0
    TA0CCR0 = 8192 - 1;     // Período de 128 Hz
    TA0CCR1 = ;         // Duty cycle de 50%

    // ALTERADO: Configura o modo de saída do canal 1, em vez da interrupção
    // OUTMOD_7: Modo Reset/Set. A saída fica alta no início e baixa ao atingir CCR1
    TA0CCTL1 = OUTMOD_7;

    // Configura o registrador de controle do Timer
    TA0CTL = TASSEL_2 | MC_1 | TACLR;

    // Loop infinito
    while(1) {
    }
}