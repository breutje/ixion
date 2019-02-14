;
; simple sample test program for ixion
;
        .include  syscall.inc                   ; system call definitions
memsize .define   $03ff                         ; top of usable memory on AVR

        .org      #$0000                        ; at reset, cpu starts PC at $0000
        move      r0,#memsize                   ; set stack to memsize
        spx       r0

        move      r0,#$0                        ; load r0 register with 0
        move      r1,#_format                   ; load address of format string
        push      r1
_loop   push      r0                            ; iteration integer
        syscall   #printf                       ; system call
        pop       r0                            ; get argument integer back

        inc       r0                            ; increment (syscalls don't modify registers)
        cmp       r0,_high                      ; compare if r0 is 0x000A (10 decimal)
        jump      nz,_loop                      ; jump to loop if not equal

        move      r1,#_bye
        push      r1
        syscall   #printf

        pop       r0                            ; (optionally) pop _bye from stack
        pop       r0                            ; (optionally) pop _format from stack

        halt

_format .string   "Loop iteration %d\n"         ; \0 terminated string
_bye    .string   "End of program\n"            ;
_high   .word     $000a                         ; 0x000A (10 decimal) constant

        .end     #$0000
