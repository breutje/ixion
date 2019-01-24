;
; group 1
;
        .org      #$0000
        inc       r0
        inc       r1
        inc       r2
        inc       r3

        dec       r0
        dec       r1
        dec       r2
        dec       r3

        shl       r0
        shl       r1
        shl       r2
        shl       r3

        shr       r0
        shr       r1
        shr       r2
        shr       r3

        spx       r0
        spx       r1
        spx       r2
        spx       r3

        push      r0
        push      r1
        push      r2
        push      r3

        pop       r0
        pop       r1
        pop       r2
        pop       r3

	.end      #$0000
