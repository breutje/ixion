//
// ixion implementation
//
#include <stdio.h>
#include <stdint.h>

#include "ixion.h"
#include "memory.h"
#include "instructions.h"

//
// registers
//
uint8_t ir, cc;
uint16_t pc, sp, r0, r1, r2, r3;

//
// global flags
//
bool execute = true;
bool running = false;
bool disassemble = true;
bool debug = false;

//
// fetch byte
//
uint8_t fetch() {
  uint8_t byte;

  byte = mm[pc++];
  if (pc >= MEMSIZE) {
    printf("\nILLEGAL MEMORY ADDRESS %04X\n", pc);
    running = false;
  }
  return byte;
}

//
// get register
//
uint16_t *get_register(uint8_t ir) {
  uint16_t *p_register = NULL;

  switch (ir & REGISTER_MASK) {
    case R0:
      p_register = &r0;
      break;
    case R1:
      p_register = &r1;
      break;
    case R2:
      p_register = &r2;
      break;
    case R3:
      p_register = &r3;
      break;
  }
  return p_register;
}

//
// syscall
//
void syscall(uint16_t call) {
  if (disassemble)
    printf("syscall(0x%04X)\n", call);
  
}

//
// get operand
//
uint16_t get_operand(uint8_t ir, uint8_t msb, uint8_t lsb) {
  uint16_t op, offset, ea;

  op = msb << 8 | lsb;

  switch (ir & 0x04) {
    case DIRECT:
      break;
    case RELATIVE:
      offset = (~op) + 1;
      ea = pc - offset;
      op = ea;
      break;
  }
  return op;
}

//
// run cpu from address
//
uint16_t ixion(uint16_t address = 0, uint16_t end = MEMSIZE - 1) {
  uint8_t msb, lsb;
  uint16_t *reg, value;
  uint16_t operand;
  char *mnemonic, *rc;

  running = true;
  pc = address;

  while (running) {
    address = pc;
    ir = fetch();
    if (disassemble)
      printf("%04X %02X ", address, ir);
    if ((ir & 0x80) != 0) { // fetch may be concurrent with instruction decoding
      msb = fetch();
      lsb = fetch();
      if (disassemble)
        printf("%02X%02X   ", msb, lsb);     
    } else if (disassemble)
      printf("       ");

    switch (ir & GROUP_MASK) {
      case GROUP0:
        switch (ir) {
          case NOP:
            mnemonic = (char *) "nop";
            break;
          case HALT:
            mnemonic = (char *) "halt";
            if (execute)
              running = false;
            break;
          case CLZ:
            mnemonic = (char *) "clz";
            if (execute)
              cc &= ~ZEROFLAG;
            break;
          case SEZ:
            mnemonic = (char *) "sez";
            if (execute)
              cc |= ZEROFLAG;
            break;
          case CLC:
            mnemonic = (char *) "clc";
            if (execute)
              cc &= ~CARRYFLAG;
            break;
          case SEC:
            mnemonic = (char *) "sec";
            if (execute)
              cc |= CARRYFLAG;
            break;
          case RESET:
            mnemonic = (char *) "reset";
            if (execute)
              pc = 0;
            break;
          case RETURN:
            mnemonic = (char *) "return";
            if (execute) {
              msb = mm[++sp];
              lsb = mm[++sp];
              pc = (msb << 8 | lsb); // POP PC
            }
            break;
          default:
            mnemonic = (char *) "ILLEGAL";
            running = false;
            break;
        }
        if (disassemble)
          printf("%-8s", mnemonic);
        break;
      case GROUP1:
        reg = get_register(ir);
        switch (ir & INSTRUCTION_MASK) {
          case INC:
            mnemonic = (char *) "inc";
            if (execute)
              (*reg)++;
            break;
          case DEC:
            mnemonic = (char *) "dec";
            if (execute)
              (*reg)--;
            break;
          case SHL:
            mnemonic = (char *) "shl";
            if (execute) {
              if (((*reg) & 0x80) == 0x80)
                cc |= CARRYFLAG; // sec
              else
                cc &= ~CARRYFLAG; // clc
              (*reg) <<= 1;
            }
            break;
          case SHR:
            mnemonic = (char *) "shr";
            if (execute) {
              if  (((*reg) & 0x01) == 0x01)
                cc |= ZEROFLAG; // sez
              else
                cc &= ~ZEROFLAG; // clz
              (*reg) >>= 1;
            }
            break;
          case SPX:
            mnemonic = (char *) "spx";
            if (execute) {
              value = (*reg);
              (*reg) = sp;
              sp = value;
            }
            break;
          case PCX:
            mnemonic = (char *) "pcx";
            if (execute) {
              value = (*reg);
              (*reg) = pc;
              pc = value;
            }
            break;
          case PUSH:
            mnemonic = (char *) "push";
            if (execute) {
              lsb = ((*reg) & 0xFF);
              msb = ((*reg) >> 8);
              mm[sp--] = lsb;
              mm[sp--] = msb;
            }
            break;
          case POP:
            mnemonic = (char *) "pop";
            if (execute) {
              msb = mm[++sp];
              lsb = mm[++sp];
              (*reg) = (msb << 8 | lsb);
            }
            break;
          default:
            mnemonic = (char *) "ILLEGAL";
            running = false;
            break;
        }
        if (disassemble)
          printf("%-8s        %s", mnemonic, register_name[ir & REGISTER_MASK]);
        break;
      case GROUP2:
        reg = get_register(ir);
        operand = get_operand(ir, msb, lsb);
        switch (ir & INSTRUCTION_MASK) {
          case LOAD:
            mnemonic = (char *) "load";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              (*reg) = (msb << 8 | lsb);
            }
            break;
          case STORE:
            mnemonic = (char *) "store";
            if (execute) {
              msb = (*reg) >> 8;
              lsb = (*reg) & 0xFF;
              mm[operand++] = msb;
              mm[operand++] = lsb;
            }
            break;
          case CMP:
            mnemonic = (char *) "cmp";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              value = (msb << 8 | lsb);
              if (value == (*reg))
                cc |= ZEROFLAG;  // sez
              else
                cc &= ~ZEROFLAG; // clz
              if (value < (*reg))
                cc |= CARRYFLAG;  // sec
              else
                cc &= ~CARRYFLAG; // clc
            }
            break;
          case ADD:
            mnemonic = (char *) "add";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              value = (msb << 8 | lsb);
              (*reg) += value;
            }
            break;
          case SUB:
            mnemonic = (char *) "sub";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              value = (msb << 8 | lsb);
              (*reg) -= value;
            }
            break;
          case AND:
            mnemonic = (char *) "and";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              value = (msb << 8 | lsb);
              (*reg) &= value;
            }
            break;
          case OR:
            mnemonic = (char *) "or";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              value = (msb << 8 | lsb);
              (*reg) |= value;
            }
            break;
          case XOR:
            mnemonic = (char *) "xor";
            if (execute) {
              msb = mm[operand++];
              lsb = mm[operand++];
              value = (msb << 8 | lsb);
              (*reg) ^= value;
            }
            break;
          default:
            mnemonic = (char *) "ILLEGAL";
            running = false;
            break;
        }
        if (disassemble)
          printf("%-8s        %s,%s%04X", mnemonic, register_name[ir & REGISTER_MASK], ((ir & MODE_MASK) == 0 ? "#" : ""), operand);
        break;
      case GROUP3:
        rc = NULL;
        reg = get_register(ir); // only used for LEA/LDI
        operand = get_operand(ir, msb, lsb);
        switch (ir & INSTRUCTION_MASK) {
          case JUMP: // conditional
            rc = (char *) condition_name[ir & CONDITION_MASK];
            mnemonic = (char *) "jump";
            if (execute) {
              switch (ir & CONDITION_MASK) {
                case NONZERO:
                  if ((cc & ZEROFLAG) == 0)
                    pc = operand;
                  break;
                case ZERO:
                  if ((cc & ZEROFLAG) == ZEROFLAG)
                    pc = operand;
                  break;
                case NOCARRY:
                  if ((cc & CARRYFLAG) == 0)
                    pc = operand;
                  break;
                case CARRY:
                  if ((cc & CARRYFLAG) == CARRYFLAG)
                    pc = operand;
                  break;
              }
            }
            break;
          case CALL: // unconditional
            mnemonic = (char *) "call";
            if (execute) {
              //
              // push PC
              //
              lsb = (pc & 0xFF);
              msb = (pc >> 8);
              mm[sp--] = lsb;
              mm[sp--] = msb;
              pc = operand;
            }
            break;
          case PASS:
            mnemonic = (char *) "pass";
            if (execute) {
              lsb = (operand & 0xFF);
              msb = (operand << 8);
              mm[sp--] = lsb;
              mm[sp--] = msb;
            }
            break;
          case MOVE:
            mnemonic = (char *) "move";
            rc = (char *) register_name[ir & REGISTER_MASK];
            if (execute)
              (*reg) = operand;
            break;
          case SYSCALL:
            mnemonic = (char *) "syscall";
            syscall(operand);
            break;
          default:
            mnemonic = (char *) "ILLEGAL";
            running = false;
            break;
        }
        if (disassemble) {
          printf("%-8s        ", mnemonic);
          if (rc != NULL)
            printf("%s,", rc);
          printf("%s%04X", ((ir & MODE_MASK) == 0 ? "#" : ""), operand);
        }
        break;
    }
    //
    // notice the instruction will be completed!
    //
    if (pc > end)
      running = false;
    printf("\n");
  }
  return 0;
}
