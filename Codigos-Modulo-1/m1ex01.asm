    .cdecls "msp430.h"
    .global main

    .text

main:
    mov     #0x2400, R4
    mov.b   #0xFE,   0(R4)
    mov.b   #0xCA,   1(R4)
    mov.w   #0x1234, 2(R4)
    mov.w   #0x5678, 4(R4)
    mov.w   #0xABCD, 5(R4)

