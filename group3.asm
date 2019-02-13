;
; group 3 (special)
;
        .org      #$0000

_begin  jump      nz,#$cade
        jump      z,#$cade
        jump      nc,#$cade
        jump      c,#$cade

        jump      nz,_begin
        jump      z,_begin
        jump      nc,_begin
        jump      c,_begin

        call      #$cade
        call      _begin

        move      r0,_begin
        move      r1,_begin
        move      r2,_begin
        move      r3,_begin

        move      r0,#$cade
        move      r1,#$cade
        move      r2,#$cade
        move      r3,#$cade

        syscall   #1

	.end      #$0000
