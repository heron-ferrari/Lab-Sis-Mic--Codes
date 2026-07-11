#include <msp430.h>

/**
 * Exercício 8: PWM com duty cycle fixo
 * Autor: [Seu Nome]
 * Descrição: Gera uma onda de 128 Hz com 50% de duty cycle no LED
 * vermelho (P1.0) usando as interrupções do Timer_A0 (CCR0 e CCR1).
 */
void main(void) {
    // 1. Parar o Watchdog Timer
    WDTCTL = WDTPW | WDTHOLD;

    // 2. Configuração do GPIO
    P1DIR |= BIT0;          // P1.0 (LED vermelho) como saída
    P1OUT &= ~BIT0;         // Garante que o LED comece desligado

    // 3. Configuração do Timer_A0
    // Período = SMCLK / 128Hz = 1048576 / 128 = 8192 ciclos
    TA0CCR0 = 8192 - 1;     // Define o período (conta até 8191)
    TA0CCR1 = 4096;         // Define o ponto do duty cycle (50%)

    // Habilita a interrupção para os canais CCR0 e CCR1
    TA0CCTL0 |= CCIE;
    TA0CCTL1 |= CCIE;

    // Configura o registrador de controle do Timer
    // TASSEL_2: Usa SMCLK como fonte de clock
    // MC_1:     Modo "Up" (conta até TA0CCR0)
    // TACLR:    Limpa o contador do timer
    TA0CTL = TASSEL_2 | MC_1 | TACLR;

    // 4. Habilitar Interrupções Globais
    // O processador agora pode responder às interrupções configuradas
    __bis_SR_register(GIE);

    // 5. Loop infinito (pode ser usado para tarefas de baixa prioridade)
    // O trabalho principal será feito pelas interrupções do Timer
    while(1) {
        // A CPU pode ser colocada em modo de baixo consumo de energia aqui,
        // pois não precisa fazer nada. Ex: __bis_SR_register(LPM0_bits);
    }
}

// --- Rotinas de Serviço de Interrupção (ISRs) ---

// ISR dedicada para o canal 0 (TA0CCR0) do Timer_A0
// O #pragma vector informa ao compilador qual evento aciona esta função [cite: 329]
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_CCR0_ISR(void) {
    // A interrupção ocorre quando o timer chega em TA0CCR0 (fim do período)
    P1OUT |= BIT0;  // Liga o LED vermelho
}

// ISR agrupada para os canais 1, 2 e overflow (TAIFG) do Timer_A0 [cite: 335]
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A0_CCR1_N_ISR(void) {
    // O registrador TA0IV nos diz qual interrupção específica ocorreu 
    switch(__even_in_range(TA0IV, TA0IV_TAIFG)) {
        case TA0IV_NONE:   break; // Nenhuma interrupção
        case TA0IV_TACCR1:        // Interrupção do canal 1 (CCR1)
            P1OUT &= ~BIT0;     // Desliga o LED vermelho
            break;
        case TA0IV_TACCR2:   break; // Interrupção do canal 2
        case TA0IV_TAIFG:    break; // Interrupção de overflow do timer
        default: break;
    }
}