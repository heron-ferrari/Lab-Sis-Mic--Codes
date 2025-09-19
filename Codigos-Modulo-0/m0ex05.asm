    .cdecls "msp430.h"
    .global main

    .text

main:
    mov.w   #(WDTPW|WDTHOLD), &WDTCTL

    mov.w   #v1,   R4     ; R4 = 0x2400
    mov.b   1(R4), R5     ; --> 2

    mov.w   #v2,   R4     ; R4 = 0x2404
    mov.b   1(R4), R6     ; --> (MSByte de 0x0001)

    mov.w   #v3,   R4     ; R4 = 0x240C
    mov.b   1(R4), R7     ; --> 0x32 (char '2')

    mov.w   #v4,   R4     ; R4 = 0x2410
    mov.b   1(R4), R8     ; --> 0x20 (char ' ')

    jmp     $
    nop

    .data ; Tudo abaixo vai p/ a RAM
v1: .byte   1 , 2 , 3 , 4
v2: .word   1 , 2 , 3 , 4
v3: .byte  '1','2','3','4'
v4: .byte  "1 , 2 , 3 , 4"
