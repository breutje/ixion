# ixion
Ixion is a simple 8-bit CPU.
It is named after the plutino [28978 Ixion](https://en.wikipedia.org/wiki/28978_Ixion).
A possible backronym could be **I**ntelligent e**X**tended **I**/**O** **N**ode as it i̶s̶  was intended to be used as intelligent I/O processor... Although [backronym.org](http://backronym.org/) reports <i>one Inept Xxii Irruption Overleaping at Nonsensicality</i> (which may be a more accurate description).

## Registers

| Register | Bits | Comment              |
| -------- | ---- | -------------------- |
| PC       | 16   | Program counter      |
| SP       | 16   | Stack pointer        |
| R0       | 16   | Register 0           |
| R1       | 16   | Register 1           |
| R2       | 16   | Register 2           |
| R3       | 16   | Register 3           |

## Registers (not directly addressable)

| Register | Bits | Comment              |
| -------- | ---- | -------------------- |
| IR       | 16   | Instruction register |
| OR       | 16   | Operand register     |
| AR       | 16   | Address register     |
| CC       | 2    | Condition codes      |

## Instruction bits

| GG | III | A | SS | Explanation  |
| -- | --- | - | -- | ------------ |
| 76 |     |   |    | Group id. if bit 7 equals 1, the instruction has a 16-bit operand |
|    | 543 |   |    | Instruction: up to 8 instructions per group |
|    |     | 2 |    | Addressing mode bit (0 = Direct/Immediate, 1 = PC Relative) |
|    |     |   | 10 | Subcode: register, condition or fixed to 00 or 11, depending on instruction |

### Instructions group 0

|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                                | Description                |
| -- | - | - | - | - | - | - | - | - | -------- | --------------------------------------- | -------------------------- |
| 00 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | NOP      | ∅                                       | No operation               |
| 08 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | HALT     | 。                                      | Halt cpu                   |
| 10 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | CLZ      | CC{Z} ← 0                               | Clear zero flag            |
| 18 | 0 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | SEZ      | CC{Z} ← 1                               | Set zero flag              |
| 20 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | CLC      | CC{C} ← 0                               | Clear carry flag           |
| 28 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 | SEC      | CC{C} ← 1                               | Set carry flag             |
| 30 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | RESET    | PC ← 0000                               | Soft reset                 |
| 38 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | RETURN   | SP ← SP + 2; PC ← [SP]                  | Return from call           |

### Instructions group 1

|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                                | Description                |
| -- | - | - | - | - | - | - | - | - | -------- | --------------------------------------- | -------------------------- |
| 40 | 0 | 1 | 0 | 0 | 0 | 0 | r | r | INC      | R ← R + 1                               | Increment register         |
| 48 | 0 | 1 | 0 | 0 | 1 | 0 | r | r | DEC      | R ← R - 1                               | Decrement register         |
| 50 | 0 | 1 | 0 | 1 | 0 | 0 | r | r | SHL      | R ← R << 1                              | Logic shift left register  |
| 58 | 0 | 1 | 0 | 1 | 1 | 0 | r | r | SHR      | R ← R >> 1                              | Logic shift right register |
| 60 | 0 | 1 | 1 | 0 | 0 | 0 | r | r | SPX      | SP ↔ R                                  | Exchange SP with register  |
| 68 | 0 | 1 | 1 | 0 | 1 | 0 | r | r | PCX      | PC ↔ R                                  | Exchange PC with register  |
| 70 | 0 | 1 | 1 | 1 | 0 | 0 | r | r | PUSH     | [SP] ← R; SP  ← SP - 2                  | Push register on stack     |
| 78 | 0 | 1 | 1 | 1 | 1 | 0 | r | r | POP      | SP ← SP + 2; R ← [SP]                   | Pop register from stack    |

### Instructions group 2

|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                                | Description                |
| -- | - | - | - | - | - | - | - | - | -------- | --------------------------------------- | -------------------------- |
| 80 | 1 | 0 | 0 | 0 | 0 | a | r | r | LOAD     | R ← [operand]                           | Load register              |
| 88 | 1 | 0 | 0 | 0 | 1 | a | r | r | STORE    | [operand] ← R                           | Store register             |
| 90 | 1 | 0 | 0 | 1 | 0 | a | r | r | CMP      | CC{Z,C}  ← R :: [operand]               | Compare register           |
| 98 | 1 | 0 | 0 | 1 | 1 | a | r | r | ADD      | R ← R + [operand]                       | Add to register            |
| A0 | 1 | 0 | 1 | 0 | 0 | a | r | r | SUB      | R ← R - [operand]                       | Subtract from register     |
| A8 | 1 | 0 | 1 | 0 | 1 | a | r | r | AND      | R ← R &amp; [operand]                   | Bitwise AND register       |
| B0 | 1 | 0 | 1 | 1 | 0 | a | r | r | OR       | R ← R &#124; [operand]                  | Bitwise OR register        |
| B8 | 1 | 0 | 1 | 1 | 1 | a | r | r | XOR      | R ← R ^ [operand]                       | Bitwise XOR register       |

### Instructions group 3
|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                                | Description                |
| -- | - | - | - | - | - | - | - | - | -------- | --------------------------------------- | -------------------------- |
| C0 | 1 | 1 | 0 | 0 | 0 | a | c | c | JUMP     | PC ← operand                            | Conditional jump           |
| C8 | 1 | 1 | 0 | 0 | 1 | a | 0 | 0 | CALL     | [SP] ← PC; SP ← SP - 2;<br>PC ← operand | Call subroutine            |
| D0 | 1 | 1 | 0 | 1 | 0 | a | r | r | MOVE     | R ← operand                             | Copy operand to register   |
| D8 | 1 | 1 | 0 | 1 | 1 | x | x | x |          |                                         |                            |
| E0 | 1 | 1 | 1 | 0 | 0 | x | x | x |          |                                         |                            |
| E8 | 1 | 1 | 1 | 0 | 1 | x | x | x |          |                                         |                            |
| F0 | 1 | 1 | 1 | 1 | 0 | x | x | x |          |                                         |                            |
| FF | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | SYSCALL  | [SP] ← PC; SP ← SP - 2;<br>TRAP operand | System call                |

## Notes ISA
* HALT is intended as a trap into a monitor. As of now, there is no mechanism to set a jump address for HLT. Also, it _may_ become dependent on the processor state, if and when implemented.
* As of now there is no user/supervisor mode (bit). This could be external (as it is necessary for a MMU implementation), but may also be implemented in the CPU itself.
* The SYSCALL is clumsy, but fixed adress jumps are undesirable. An extra register will cost at least 40 extra gates.
* SYSCALL/HALT: It may be desireable to do implement an extra register. HALT could jump there and SYSCALL could use offsets (reuse of register bits or even add 'a' bit for 8 separate syscalls).
* CC may be extended with user/supervisor bit and/pr processor state.
* There is no register renaming. Providing an alternate set of registers would explode the gate count.
* The instruction set was defined to fit into a single byte (hence 8-bit CPU).

## Notes hardware
* The CPU is implemented using micro schedulers. e.g. the fetcher is a simple state machine fetching 3 bytes (1 byte to instruction register + (optionally) 2 argument bytes to operand register).
* The ALU is either 8 bit with micro scheduler or 16 bit direct.
* The address generation unit output is the address output register.
* MUL/DIV should be implemented in software.
* There is no DR (data register) as registers output their content directly to the output data bus during the right time cycle.
* There is no pipeline, nor is ixion superscalar (but could be at the cost of exploding gate count).
