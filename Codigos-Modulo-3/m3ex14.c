#include <msp430.h>
#include <stdint.h>

/*
 * Exercício 14: Controle de LEDs via Joystick (PWM via Software)
 * Hardware: MSP-EXP430F5529LP
 *
 * Solução para o erro anterior:
 * Como P1.0 e P4.7 não possuem saída direta de PWM por hardware nesta placa,
 * usamos interrupções do Timer A0 para fazer o chaveamento manual dos pinos.
 */

volatile uint8_t eixo_x = 0;
volatile uint8_t eixo_y = 0;

// Função de Leitura do ADC (8 bits)
uint8_t adcRead(uint8_t canal) {
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_8 | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL2 = ADC12RES_0; // 8 bits (0-255)
    ADC12MCTL0 = canal;
    ADC12CTL0 |= ADC12ENC | ADC12SC;
    while (ADC12CTL1 & ADC12BUSY);
    return (uint8_t)ADC12MEM0;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    // --- Configuração dos LEDs (GPIO Normal) ---
    // Importante: PxSEL deve ser 0 para controlarmos via software
    P1DIR |= BIT0;   // P1.0 Saída (Vermelho)
    P1SEL &= ~BIT0;  // Garante função GPIO
    P1OUT &= ~BIT0;

    P4DIR |= BIT7;   // P4.7 Saída (Verde)
    P4SEL &= ~BIT7;  // Garante função GPIO
    P4OUT &= ~BIT7;

    // --- Configuração do ADC ---
    P6SEL |= BIT0 | BIT1; // P6.0 e P6.1 como analógicos

    // --- Configuração do Timer A0 para PWM via Software ---
    // Objetivo: PWM ~100Hz com 256 passos.
    // Clock Alvo do Timer = 100Hz * 256 = 25.600 Hz.
    // SMCLK padrao = ~1MHz.
    // Divisor total necessário = ~40. (1MHz / 40 = 25kHz)

    TA0CCR0 = 255; // Período do PWM (rola de 0 a 255)

    // Duty Cycles Iniciais (serão atualizados no loop)
    TA0CCR1 = 0;   // Comparador para LED Vermelho
    TA0CCR2 = 0;   // Comparador para LED Verde

    // Configurar Clock: SMCLK, Divisor /8, ExDivisor /5 = /40
    TA0EX0 = TAIDEX_4; // Divide por 5
    TA0CTL = TASSEL_2 | ID_3 | MC_1 | TACLR | TAIE;
    // TASSEL_2 (SMCLK) | ID_3 (/8) | MC_1 (Up Mode) | Clear | Hab. Int. Overflow

    // Habilitar interrupções dos comparadores CCR1 e CCR2
    TA0CCTL1 = CCIE;
    TA0CCTL2 = CCIE;
    TA0CCTL0 = CCIE; // Habilitar interrupção do CCR0 (início do ciclo)

    __enable_interrupt(); // Habilitar interrupções globais

    while(1) {
        // Ler Eixos
        eixo_x = adcRead(ADC12INCH_0);
        eixo_y = adcRead(ADC12INCH_1);

        // Atualizar os comparadores do Timer
        // O Timer roda em background. Aqui só atualizamos o ponto de corte.
        TA0CCR1 = eixo_x;
        TA0CCR2 = eixo_y;

        // Delay de amostragem (~20ms)
        __delay_cycles(20000);
    }
}

// --- Rotinas de Interrupção (Onde a mágica acontece) ---

// 1. Interrupção do CCR0 (Acontece quando o contador chega em 255/reset)
// Usada para ligar os LEDs no início de cada ciclo PWM
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
    // No início do ciclo, ligamos os LEDs (se o duty não for 0)
    if (eixo_x > 0) P1OUT |= BIT0;
    if (eixo_y > 0) P4OUT |= BIT7;
}

// 2. Interrupção Geral do Timer (CCR1, CCR2 e Overflow)
// Usada para desligar os LEDs quando o tempo do duty cycle for atingido
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
    switch(__even_in_range(TA0IV, 14)) {
        case 2:  // Interrupção do CCR1 (LED Vermelho)
            // Tempo ligado acabou, apaga o LED
            if (eixo_x < 255) P1OUT &= ~BIT0;
            break;

        case 4:  // Interrupção do CCR2 (LED Verde)
            // Tempo ligado acabou, apaga o LED
            if (eixo_y < 255) P4OUT &= ~BIT7;
            break;

        default: break;
    }
}