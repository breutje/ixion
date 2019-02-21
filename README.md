# ixion
Ixion is a simple 8-bit CPU.
It is named after the plutino [28978 Ixion](https://en.wikipedia.org/wiki/28978_Ixion).
A possible backronym could be **I**ntelligent e**X**tended **I**/**O** **N**ode as it is intended to be used as intelligent I/O processor... Although [backronym.org](http://backronym.org/) reports <i>one Inept Xxii Irruption Overleaping at Nonsensicality</i> (which may be a more accurate description).

## Registers

| Register | Bits | Comment         |
| -------- | ---- | --------------- |
| PC       | 16   | Program counter |
| SP       | 16   | Stack pointer   |
| R0       | 16   | Register 0      |
| R1       | 16   | Register 1      |
| R2       | 16   | Register 2      |
| R3       | 16   | Register 3      |
| CC       | 8    | Condition codes |

## Instruction bits

| GG | III | A | SS | Explanation  |
| -- | --- | - | -- | ------------ |
| 76 |     |   |    | Group id. if bit 7 equals 1, the instruction has a 16-bit operand |
|    | 543 |   |    | Instruction: up to 8 instructions per group |
|    |     | 2 |    | Addressing mode bit (0 = Direct/Immediate, 1 = PC Relative) |
|    |     |   | 10 | Subcode: register, condition or fixed to 00 or 11, depending on instruction |

## Instructions group 0

|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                | Description                                          |
| -- | - | - | - | - | - | - | - | - | -------- | ----------------------- | ---------------------------------------------------- |
| 00 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | NOP      | ∅                       | No operation                                         |
| 08 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | HALT     | 。                      | Halt cpu                                             |
| 10 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | CLZ      | CC{Z} ← 0               | Clear zero flag                                      |
| 18 | 0 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | SEZ      | CC{Z} ← 1               | Set zero flag                                        |
| 20 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | CLC      | CC{C} ← 0               | Clear carry flag                                     |
| 28 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 | SEC      | CC{C} ← 1               | Set carry flag                                       |
| 30 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | RESET    | PC ← 0000               | Soft reset                                           |
| 38 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | RETURN   | PC ← [SP]; SP ← SP + 2  | Return from call                                     |

## Instructions group 1

|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                | Description                                          |
| -- | - | - | - | - | - | - | - | - | -------- | ----------------------- | ---------------------------------------------------- |
| 40 | 0 | 1 | 0 | 0 | 0 | 0 | r | r | INC      | R ← R + 1               | Increment register                                   |
| 48 | 0 | 1 | 0 | 0 | 1 | 0 | r | r | DEC      | R ← R - 1               | Decrement register                                   |
| 50 | 0 | 1 | 0 | 1 | 0 | 0 | r | r | SHL      | R ← R << 1              | Logic shift left register                            |
| 58 | 0 | 1 | 0 | 1 | 1 | 0 | r | r | SHR      | R ← R >> 1              | Logic shift right register                           |
| 60 | 0 | 1 | 1 | 0 | 0 | 0 | r | r | SPX      | SP ↔ R                  | Exchange SP with register                            |
| 68 | 0 | 1 | 1 | 0 | 1 | 0 | r | r | PCX      | PC ↔ R                  | Exchange PC with register                            |
| 70 | 0 | 1 | 1 | 1 | 0 | 0 | r | r | PUSH     | [SP] ← R; SP  ← SP - 2  | Push register on stack                               |
| 78 | 0 | 1 | 1 | 1 | 1 | 0 | r | r | POP      | SP ← SP + 2; R ← [SP]   | Pop register from stack                              |

## Instructions group 2
|    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Mnemonic | Function                | Description                                          |
| -- | - | - | - | - | - | - | - | - | -------- | ----------------------- | ---------------------------------------------------- |
| 80 | 1 | 0 | 0 | 0 | 0 | a | r | r | LOAD     | R ← [operand]           | Load register from [EA]                 |
| 88 | 1 | 0 | 0 | 0 | 1 | a | r | r | STORE    | [operand] ← R           | Store register to [EA]                  |
| 90 | 1 | 0 | 0 | 1 | 0 | a | r | r | CMP      | CC{}  ← R O [operand]  | Compare register with [EA]              |
| 98 | 1 | 0 | 0 | 1 | 1 | a | r | r | ADD      | R ← R + [operand]       | Add data from [EA] to register         |
| A0 | 1 | 0 | 1 | 0 | 0 | a | r | r | SUB      | R ← R - [operand]       | Subtract data from [EA] from register   |
| A8 | 1 | 0 | 1 | 0 | 1 | a | r | r | AND      | R ← R &amp; [operand]       | Bitwise register AND [EA] |
| B0 | 1 | 0 | 1 | 1 | 0 | a | r | r | OR       | R ← R &#124; [operand]  | Bitwise register OR [EA] |
| B8 | 1 | 0 | 1 | 1 | 1 | a | r | r | XOR      | R ← R ^ [operand]       | Bitwise register XOR [EA] |
