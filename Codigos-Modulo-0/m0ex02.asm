    .cdecls "msp430.h"
    .global main

    .text

main:
    mov.w   #(WDTPW|WDTHOLD), &WDTCTL

    mov.b   #7, R4    ; Inicializa R4 = 7
    mov.b   #9, R5    ; Inicializa R5 = 9
    add.b   R4, R5    ; R5 = R4 + R5

    jmp     $
    nop
