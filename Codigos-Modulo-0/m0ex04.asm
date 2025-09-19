    .cdecls "msp430.h"
    .global main

    .text

main:
    mov.w   #(WDTPW|WDTHOLD), &WDTCTL

    mov     #3, R4    ; Inicializa R4 = 3
    mov     #4, R5    ; R5 = 4
    clr     R6        ; R6 = 0

main_loop:
    call    #acumula  ; Chama acumula
    dec     R4        ; R4 vezes
    jnz     main_loop

    jmp     $         ; Resultado:
    nop               ; R6 = R4 x R5
  
; ------------------------------------
; Subrotina que acumula R5 em R6
acumula:
    add     R5, R6    ; R6 = R6 + R5
    ret