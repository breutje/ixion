;
; group 2
;
        .org      #$0000

_begin  load      r0,#$cade
        load      r1,#$cade
        load      r2,#$cade
        load      r3,#$cade

        load      r0,_begin
        load      r1,_begin
        load      r2,_begin
        load      r3,_begin

        store     r0,#$cade
        store     r1,#$cade
        store     r2,#$cade
        store     r3,#$cade

        store     r0,_begin
        store     r1,_begin
        store     r2,_begin
        store     r3,_begin

        cmp       r0,#$cade
        cmp       r1,#$cade
        cmp       r2,#$cade
        cmp       r3,#$cade

        cmp       r0,_begin
        cmp       r1,_begin
        cmp       r2,_begin
        cmp       r3,_begin

        add       r0,#$cade
        add       r1,#$cade
        add       r2,#$cade
        add       r3,#$cade

        add       r0,_begin
        add       r1,_begin
        add       r2,_begin
        add       r3,_begin

        sub       r0,#$cade
        sub       r1,#$cade
        sub       r2,#$cade
        sub       r3,#$cade

        sub       r0,_begin
        sub       r1,_begin
        sub       r2,_begin
        sub       r3,_begin

        and       r0,#$cade
        and       r1,#$cade
        and       r2,#$cade
        and       r3,#$cade

        and       r0,_begin
        and       r1,_begin
        and       r2,_begin
        and       r3,_begin

        or        r0,#$cade
        or        r1,#$cade
        or        r2,#$cade
        or        r3,#$cade

        or        r0,_begin
        or        r1,_begin
        or        r2,_begin
        or        r3,_begin

        xor       r0,#$cade
        xor       r1,#$cade
        xor       r2,#$cade
        xor       r3,#$cade

        xor       r0,_begin
        xor       r1,_begin
        xor       r2,_begin
        xor       r3,_begin

	.end      #$0000
