# Plan
The things to do before ixion can be implemented in hardware.

## SYSCALL
The decision has been made: `HALT` (0x08) is replaced by a `SYSCALL0` (0x08) to SYSCALL7 (0x0F).
The vector table has not been established yet, but could be either fixed or defined later.
`HALT` itself can be one of the SYSCALLs. If a SYSCALL should require a register argument, that is still possible, but now pure software.

## RESET
As `RESET` is really a system function, it does not belong in user space.
It should be removed and eventually replaced by a `SYSCALL`.
Historic note: it was analogous to the 6502's `BRK` instruction.

## Plan
* Update the README.md
* Update the emulator (check genixion).
* update the assembler. Modernize and make it 2-pass.
* Write an assembler priner / guide.
* Implement test 7 feasibility programs

## Feasibility programs
The goal is to write real-world programs to check if the ISA will stand up to real coding.
The multiplication and division code could be the start of useful library.

* **Fibonacci Series**: exercises basic register moves,
  addition, conditional branching, loop counters,
  and integer overflow flags without needing complex memory access.
* **8-bit Multiplication**: register pressure and flag behaviour.
* **8-bit Division**: register pressure, exception and flag behaviour.
* **Sieve of Eratosthenes**: Ultimate benchmark! It tests indexed memory addressing,
  bitwise operations/masking, nested loops, and memory bandwidth.

