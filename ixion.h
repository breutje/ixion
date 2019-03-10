//
// ixion definitions
//
#define READ_BUFFER     1024
#define CHUNK             16

//
// version and build
//
extern char *_version;
extern int _build;
extern char *_date;

//
// cpu
//
extern uint16_t pc;
extern uint16_t sp;
extern uint16_t r0;
extern uint16_t r1;
extern uint16_t r2;
extern uint16_t r3;
extern uint8_t cc;

//
// flags
//
extern bool disassemble;
extern bool execute;
extern bool debug;

//
// instruction
//
struct _instruction {
  uint8_t code;
  const char *mnemonic;
};

//
// instructions, registers and conditions
//
extern struct _instruction instructions[];
extern const char *register_name[];
extern const char *condition_name[];
extern unsigned char translate[];

//
// registers (direct encoding)
//
#define R0                0x00  // 0b00000000
#define R1                0x01  // 0b00000001
#define R2                0x02  // 0b00000010
#define R3                0x03  // 0b00000011
#define REGISTER_MASK     0x03  // 0b00000011
#define UNKNOWN_REGISTER  0xFF  // 0b11111111

//
// conditions (direct encoding) for JUMP
//
#define NONZERO           0x00  // 0b00000000 NZ
#define ZERO              0x01  // 0b00000001 Z
#define NOCARRY           0x02  // 0b00000010 NC
#define CARRY             0x03  // 0b00000011 C
#define CONDITION_MASK    0x03  // 0b00000011
#define UNKNOWN_CONDITION 0xFF  // 0b11111111

//
// addressing modes (direct encoding) no INDEXED
//
#define DIRECT            0x00  // 0b00000000
#define RELATIVE          0x04  // 0b00000100
#define MODE_MASK         0x04  // 0b00000100

#define DEFINED           0x01  // used in assembler for labels
#define PREDEFINED        0x02  // used in assembler for include definitions

//
// cc register flags
//
#define ZEROFLAG          0x01  // 0b00000001
#define CARRYFLAG         0x02  // 0b00000010

//
// instruction group
//
#define GROUP0            0x00
#define GROUP1            0x40
#define GROUP2            0x80
#define GROUP3            0xC0
#define GROUP_MASK        0xC0

#define INSTRUCTION_MASK  0xF8

//
// prototypes
//
uint16_t ixion(uint16_t address, uint16_t end);
int disixion(int argc, char *argv[]);
int asmixion(int argc, char *argv[]);
int monixion(int argc, char *argv[]);

