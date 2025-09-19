;-------------------------------------------------------------------------------
;                       Laboratório de Sistemas Microprocessados
;                       Exercício 21: Ordenação de Vetor (Bubble Sort)
;
; Descrição: Sub-rotina 'ORDENA' que ordena um vetor de bytes em ordem
;            crescente usando o método da bolha.
;-------------------------------------------------------------------------------
        .cdecls "msp430.h"
        .global main

        .text

;-------------------------------------------------------------------------------
; Rotina Principal
;-------------------------------------------------------------------------------
main:
            mov.w   #0x0280, SP                 ; Inicializa o Stack Pointer
            mov.w   #WDTPW | WDTHOLD, &WDTCTL   ; Para o Watchdog Timer

            ; Prepara os parâmetros para a sub-rotina ORDENA
            mov.w   #vetor_teste, R12           ; R12 <- Endereço do vetor
            mov.w   #TAMANHO, R13               ; R13 <- Tamanho do vetor

            call    #ORDENA                     ; Chama a sub-rotina de ordenação

            jmp     $                           ; Trava a execução ao final

;-------------------------------------------------------------------------------
; SUB-ROTINA: ORDENA
; Parâmetros de Entrada: R12 -> Endereço do vetor de bytes
;                        R13 -> Tamanho do vetor
; Registros utilizados: R4, R6, R7, R14, R15
;-------------------------------------------------------------------------------
ORDENA:
            ; Salva os registros utilizados na pilha
            PUSH    R4
            PUSH    R6
            PUSH    R7
            PUSH    R14
            PUSH    R15

            ; Configura o contador do laço externo (N-1 passadas)
            mov.w   R13, R14
            dec.w   R14                         ; R14 = N-1
            jz      ordena_exit                 ; Se N <= 1, não há nada a ordenar

outer_loop:
            ; Configura o laço interno
            mov.w   R14, R15                    ; Contador do laço interno (percorre N-1, N-2, ...)
            mov.w   R12, R4                     ; R4 é o ponteiro, reinicia a cada passada

inner_loop:
            ; Carrega elementos vizinhos
            mov.b   @R4, R6                     ; R6 = vetor[j]
            mov.b   1(R4), R7                   ; R7 = vetor[j+1]

            ; Compara R7 com R6 (R7 - R6). Se R7 >= R6, o Carry flag é 1.
            cmp.b   R6, R7
            jc      no_swap                     ; Pula a troca se R7 >= R6 (ordem correta)

            ; Troca os bytes na memória se R7 < R6
            mov.b   R7, 0(R4)
            mov.b   R6, 1(R4)

no_swap:
            inc.w   R4                          ; Avança o ponteiro para o próximo par
            dec.w   R15                         ; Decrementa o contador do laço interno
            jnz     inner_loop                  ; Repete o laço interno

            dec.w   R14                         ; Decrementa o contador do laço externo
            jnz     outer_loop                  ; Inicia a próxima passada

ordena_exit:
            ; Restaura os registros da pilha
            POP     R15
            POP     R14
            POP     R7
            POP     R6
            POP     R4
            ret

;-------------------------------------------------------------------------------
; Segmento de Dados
;-------------------------------------------------------------------------------
                .data
TAMANHO         .set    8                   ; Define o tamanho do vetor como uma constante
vetor_teste:    .byte   4, 7, 3, 5, 1, 3, 2, 6     ; Vetor de exemplo [cite: 351]

                .end