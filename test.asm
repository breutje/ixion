;
; simple sample test program for ixion
;
        .include  syscall.inc                   ; system call definitions

        .org      #$0000                        ; default

        move      r0,#$03ff                     ; set stack to memsize
        spx       r0

        move      r0,#$0                        ; load r0 register with 0
        pass      _format                       ; push [ea] of format string on stack (stays there :-)
_loop   push      r0                            ; iteration integer
        syscall   #1                            ; printf
        pop       r0                            ; get argument integer back

        inc       r0                            ; increment (syscalls don't modify registers)
        cmp       r0,#$000a                     ; compare if D is 0x000A (10 decimal)
        jump      nz,_loop                      ; jump to loop if not equal

        pass      _bye
        syscall   #printf

        pop       r0                            ; (optionally) pop _bye from stack
        pop       r0                            ; (optionally) pop _format from stack

        halt                                    ; notice _format is still on the stack

_format .string   "Loop iteration %d\n"         ; \0 terminated string
_bye    .string   "End of program\n"            ;


        .end     #$0000
