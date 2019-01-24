# ixion

+---------+
|    PC   |     can only be modified by RET, RST, JUMP, CALL and SYSCALL
+---------+
|    SP   |     can only be modified by PUSH, POP, PASS and SPX (exchange with register)
+---------+
|    R0   |     general purpose register 0
+---------+
|    R1   |     general purpose register 1
+---------+
|    R2   |     general purpose register 2
+---------+
|    R3   |     general purpose register 3
+----+----+
     | CC |     condition code register (8 bits, only 2 are used: CARRY and ZERO flags)
     +----+
