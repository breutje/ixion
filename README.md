# ixion
Ixion is a simple 8-bit CPU. It is based on previous designs: cpuz8, scx and scx32.
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

# Instructions

| GG | III | A | SS | Explanation  |
| -- | --- | - | -- | ------------ |
| 76 |     |   |    | Group id. if bit 7 equals 1, the instruction has a 16-bit operand |
|    | 543 |   |    | Instruction: up to 8 instructions per group |
|    |     | 2 |    | Addressing mode bit (0 = Direct/Immediate, 1 = PC Relative) |
|    |     |   | 10 | Subcode: register, condition or fixed to 00 or 11, depending on instruction |
