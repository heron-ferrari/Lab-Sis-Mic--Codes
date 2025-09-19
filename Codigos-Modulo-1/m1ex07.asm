      .cdecls "msp430.h"
      .global main

      .text

main:
      mov.w   #(WDTPW|WDTHOLD), &WDTCTL  ; Para o Watchdog Timer

      ; Inicialização dos parâmetros para a sequência
      mov.w   #0x2400, R4   ; R4 = Ponteiro para o endereço de memória
      mov.w   #18, R5       ; R5 = Contador (18 pois os 2 primeiros são casos especiais)
      clr.w   R6            ; R6 = F(n-2), inicializado com 0
      mov.w   #1, R7        ; R7 = F(n-1), inicializado com 1

      ; Armazena os dois primeiros números (casos base)
      mov.w   R6, 0(R4)     ; Armazena F(0) = 0
      add.w   #2, R4        ; Avança ponteiro
      mov.w   R7, 0(R4)     ; Armazena F(1) = 1
      add.w   #2, R4        ; Avança ponteiro

main_loop:
      call    #FIB          ; Chama a sub-rotina para calcular e armazenar UM número
      dec.w   R5            ; Decrementa o contador
      jnz     main_loop     ; Se o contador não for zero, repete

      jmp     $             ; Trava a execução em um laço infinito após a conclusão

FIB:
      ; R8 é usado como registrador temporário para o resultado
      mov.w   R7, R8        ; R8 = F(n-1)
      add.w   R6, R8        ; R8 = F(n-1) + F(n-2) -> Novo número

      mov.w   R8, 0(R4)     ; Armazena o novo número no endereço apontado por R4
      add.w   #2, R4        ; Avança o ponteiro de memória para a próxima posição

      ; Atualiza os valores para a próxima iteração
      mov.w   R7, R6        ; O antigo F(n-1) se torna o novo F(n-2)
      mov.w   R8, R7        ; O novo F(n) se torna o novo F(n-1)

      ret                   ; Retorna da sub-rotina