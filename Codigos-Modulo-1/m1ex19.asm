        .cdecls C,LIST,"msp430.h"
        .global main
        .text

;-------------------------------------------------------------------------------
main:
        mov.w   #(WDTPW|WDTHOLD), &WDTCTL   ; Desabilita Watchdog

        mov     #0x89AB, R12    ; Número de 16 bits para converter
        mov     #saida,  R13    ; Endereço de saída

        call    #W16_ASC        ; Chama sub-rotina

        jmp     $               ; Loop infinito
        nop

;-------------------------------------------------------------------------------
; Sub-rotina W16_ASC
; Entrada:
;   R12 = valor 16 bits
;   R13 = endereço de saída
; Saída:
;   @R13..@R13+3 = ASCII de cada nibble (MSB → LSB)
;-------------------------------------------------------------------------------

W16_ASC:
        push    R4
        push    R5

        mov     R13, R5         ; ponteiro de escrita

        ; --- nibble 1 (bits 15-12)
        mov     R12, R4
        rra     R4
        rra     R4
        rra     R4
        rra     R4              ; >> 4
        rra     R4
        rra     R4
        rra     R4
        rra     R4              ; >> 8
        rra     R4
        rra     R4
        rra     R4
        rra     R4              ; >> 12
        and     #0x000F, R4
        call    #NIB_ASC
        mov.b   R4, 0(R5)
        inc     R5

        ; --- nibble 2 (bits 11-8)
        mov     R12, R4
        rra     R4
        rra     R4
        rra     R4
        rra     R4              ; >> 4
        rra     R4
        rra     R4
        rra     R4
        rra     R4              ; >> 8
        and     #0x000F, R4
        call    #NIB_ASC
        mov.b   R4, 0(R5)
        inc     R5

        ; --- nibble 3 (bits 7-4)
        mov     R12, R4
        rra     R4
        rra     R4
        rra     R4
        rra     R4              ; >> 4
        and     #0x000F, R4
        call    #NIB_ASC
        mov.b   R4, 0(R5)
        inc     R5

        ; --- nibble 4 (bits 3-0)
        mov     R12, R4
        and     #0x000F, R4
        call    #NIB_ASC
        mov.b   R4, 0(R5)

        pop     R5
        pop     R4
        ret

;-------------------------------------------------------------------------------
; Sub-rotina NIB_ASC
; Entrada: R4 = nibble (0-15)
; Saída:   R4 = caractere ASCII ('0'-'9', 'A'-'F')
;-------------------------------------------------------------------------------
NIB_ASC:
        cmp     #10, R4
        jl      digito
        add     #55, R4        ; 10 -> 'A' (0x41)
        ret
digito: add     #48, R4        ; 0 -> '0' (0x30)
        ret


        .data
saida:  .space 4
