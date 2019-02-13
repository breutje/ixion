ixion - simple cpu and assembler based on cpuz8, scx and scx32, named after the plutino 28978 Ixion
===================================================================================================
- 64k addressing, but 1024 bytes should be enough for AVR implementation
- all instructions are 8 bit.
- all instructions are directly encoded.
- bit 7 determines if there is an operand.
- only 2 addressing modes: direct/immediate and relative.
- only 2 condition flags (zero and carry).
- register - memory architecture with memory - stack extension
- Conditional JUMP, unconditional CALL
- no R ￩ R transfers (exchange) execept for SP
- big endian (first operand byte followig instruction is msb)
- s̶o̶m̶e̶ all code needs modernizing ;-)
- macro's directly encode the opcode (cpuz8 used union/struct/bitfield)
- all unimplemented or illegal instructions are trapped (wish rather than contract).
- implemented in C, not C++
- assembler language is not well specified and is a big simplified confusion of AVR, ARM, Z80, PDP and 6809 styles.....
- SYSCALL is executed by the monitor (host), not the ixion cpu
- adapted for conservative handling of sequence points
- sp points to first free location (push is post decrement)

cpu
===
Register model:

+---------+
|    PC   |     can only be modified by RET, RST, JUMP, CALL and SYSCALL
+---------+
|    SP   |     can only be modified by PUSH, POP, and SPX (exchange with register)
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

instructions
============

assembler
=========
- really simple single pass in-memory assembler with S1/S9 save option
- standalone and integrated in monitor

monitor
=======
- simple instructions
- loads/saves s1/s9 records
- required/optional case insensitive keywords (eg. 'Load' or 'l' is equivalent)
- required encoded in uppercase, optional part in lowercase
- simple command history using up and down arrows (vt100)
- minimal version for AVR implementation

disassembler
============
- really simple
- reads S1/S9 file to memory, then lists memory
- both standalone and integrated in monitor

programmer
=========
- no definition of I/O of programmer (yet). Possibly the syscalls may be used for that.
- programmer is supposed to be part of monitor, not ixion cpu
- ixion cpu used for device specific algorithms

name
====
Ixion was really named after the plutino 28978 Ixion
But: Intelligent eXtended I/O Node
     -            -       - - -

As ixion IS intended to do the I/O for a front-end application...

