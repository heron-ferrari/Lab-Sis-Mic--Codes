    .cdecls "msp430.h"
    .global main

    .text

main:
    mov.w   #vetor, R4
    mov.b   @R4+,   R5
    mov.b   @R4+,   R6
    mov.w   @R4+,   R7
    mov.w   @R4+,   R8
    mov.b   #0x33,  &var8


        .data
vetor:  .byte   0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE
var8:   .byte   0x00