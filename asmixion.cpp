//
// asmixion -  simple ixion assembler (non retargetable) output to memory (and S1/S9)
//
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libgen.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>

#include "s19.h"
#include "ixion.h"
#include "memory.h"
#include "instructions.h"

#define MAX_LABEL           32
#define MAX_SYMBOLS        256

#define UNDEFINED       "----"

//
// prototypes
//
int include(char *filename);
uint8_t register_bits(char *register_name);
uint8_t condition_bits(char *condition_name);
uint16_t operand_value(char *input);
char *label_value(char *label, int max, int mode, uint16_t address);
int getLine(char line[], int maxlen, FILE *stream, bool  history);

//
// version and build
//
extern char *_version;
extern int _build;
extern char *_date;

//
// global symbol and reference tables
//
struct {
  char label[MAX_LABEL + 1];
  uint16_t value; 
  int mode;
  int lineno;
} symbols[MAX_SYMBOLS], reference[MAX_SYMBOLS];

int labelno = 0;

//
// asmixion <file>
//
int asmixion(int argc, char *argv[]) {
  FILE *in;
  int errors = 0, warnings = 0, l, m, n, i, group, referenceno = 0, lineno = 0;
  bool found;
  char *dir, *base, *dot, valuestr[MAX_LABEL + 1], operand[MAX_LABEL + 1], input[READ_BUFFER], output[READ_BUFFER];
  char bytes, sstart[5], send[5], stransfer[5], *sargv[6];
  char c, asmline[READ_BUFFER + 1];
  char mnemonic[42];
  char *p, *s, *label, *instruction, *arguments, *aregister, *avalue;
  uint8_t code, byte, bits, condition, mode, msb, lsb;
  uint16_t mem, start, end, address, transfer, value, word;

  if (argc != 2) {
    printf("Error: Usage: asmixion <filename>\n");
    return -1;
  }

  printf("%s.%d\n", _version, _build);

  //
  // construct intput and output filename
  //
  strcpy(input, argv[1]);
  base = basename(argv[1]);
  dir = dirname(argv[1]);
  if ((dot = strrchr(base, '.')) != NULL)
    *dot = '\0';
  strcpy(output, dir);
  strcat(output, "/");
  strcat(output, base);
  strcat(output, ".s19");

  //
  // Assemble...
  //
  if ((in = fopen(input, "r")) == NULL) {
    printf("Error: cannot open '%s' for reading\n", argv[1]);
    return -1;
  }
  start = 0;
  address = 0;
  while (getLine(asmline, READ_BUFFER, in, false) != EOF) {
    lineno++;
    c = asmline[0];
    p = &asmline[0];

    //
    // skip lines without commands
    //
    if ((c == ';') || (strlen(asmline) == 0))
      continue;

    //
    // up to 3 fields:  [label] instruction|directive arguments (may be a comma separated list)
    //
    label = instruction = arguments = NULL;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
      label = strtok(p, " ;");
      strcpy(symbols[labelno].label, label);
      symbols[labelno].value = address;
      symbols[labelno].lineno = lineno;
      symbols[labelno].mode = DEFINED;
      labelno++;
      p = NULL;
    }
    if ((instruction = strtok(p, " ;")) == NULL) {
      printf("Error: syntax error in line #%d (expected instruction or directive)\n", lineno);
      fclose(in);
      return -2;
    }

    if (*instruction == '.') {
      //
      // directive
      //
      if (strcasecmp(instruction, ".org") == 0) {
        if ((arguments = strtok(NULL, " ;")) != NULL) {
          if (*arguments != '#') {
            printf("Error: error in line #%d (only immedate arguments are allowed)\n", lineno);
            fclose(in);
            return -2;
          }
          address = operand_value(++arguments);
        } else
          address = 0;
      } else if (strcasecmp(instruction, ".end") == 0) {
        if ((arguments = strtok(NULL, " ;")) != NULL) {
          if (*arguments != '#') {
            printf("Error: error in line #%d (only immedate arguments are allowed)\n", lineno);
            fclose(in);
            return -2;
          }
          transfer = operand_value(++arguments);
        } else
          transfer = 0;
      } else if (strcasecmp(instruction, ".string") == 0) {
        do
          arguments = strtok(NULL, "\"");
        while (*arguments == ' ');
        for (n = 0; n < (int) strlen(arguments); n++) {
          if (n == 0)
            printf("%04d: %04X", lineno, address);
          else if ((n % 4) == 0)
            printf("\n%04d: %04X", lineno, address);
          printf(" %02X", arguments[n]);
          mm[address++] = (uint8_t) arguments[n];
        }
        printf("\n%04d: %04X 00\n", lineno, address);
        mm[address++] = (uint8_t) 0;
      } else if (strcasecmp(instruction, ".word") == 0) {
        arguments = strtok(NULL, "; ");
        for (n = 0, p = strtok(arguments, ", ;"); p != NULL; n++, p = strtok(NULL, ", ;")) {
          word = (uint16_t) operand_value(p);
          msb = (word & 0xFF00) >> 8;
          lsb = word & 0x00FF;
          printf("%04d: %04X %02X %02X\n", lineno, address, msb, lsb);
          mm[address++] = msb;
          mm[address++] = lsb;
        }
        printf("\n");
      } else if (strcasecmp(instruction, ".byte") == 0) {
        arguments = strtok(NULL, "; ");
        for (n = 0, p = strtok(arguments, ", ;"); p != NULL; n++, p = strtok(NULL, ", ;")) {
          byte = (uint8_t) operand_value(p);
          printf("%04d: %04X %02X\n", lineno, address, byte);
          mm[address++] = byte;
        }
        printf("\n");
      } else if (strcasecmp(instruction, ".define") == 0) {
        //
        // fix just entered label with value defined here (!)
        //
        arguments = strtok(NULL, "; ");
        value = operand_value(++arguments);
        symbols[labelno - 1].value = value;
      } else if (strcasecmp(instruction, ".include") == 0) {
        arguments = strtok(NULL, "; ");
        errors += include(arguments);
      } else {
        printf("Error: syntax error in line #%d (unknown or unimplemented directive: \"%s\")\n", lineno, instruction);
        fclose(in);
        return -2;
      }
    } else {

      //
      // instruction
      //
      arguments = strtok(NULL, " ;");

      //
      // find instruction
      //
      for (n = 0; instructions[n].mnemonic != NULL; n++) {
        if (strcasecmp(instructions[n].mnemonic, instruction) == 0)
          break;
      }
      if (instructions[n].mnemonic == NULL) {
        printf("Error: syntax error in line #%d (unknown or unimplemented instruction: \"%s\")\n", lineno, instruction);
        fclose(in);
        return -2;
      }

      //
      // mnemonic, code, group
      //
      strcpy(mnemonic, instructions[n].mnemonic);
      code = instructions[n].code;
      group = (code & 0xc0) >> 6;

      switch (group) {
        case 0: // GROUP 0 (no operand, implied)
          printf("%04d: %04X %02X              %-12s\n", lineno, address, code, mnemonic);
          mm[address++] = code;
          break;
        case 1: // GROUP 1 (no operand, register)
          bits = register_bits(arguments);
          if (bits == UNKNOWN_REGISTER) {
            printf("Error: unknown register \"%s\" in line #%d\n", arguments, lineno);
            fclose(in);
            return -2;
          }
          code |= bits;
          printf("%04d: %04X %02X              %-12s     %s\n", lineno, address, code, mnemonic, register_name[bits]);
          mm[address++] = code;
          break;
        case 2: // GROUP 2 (16 bit operand, addressing mode + register)
          if ((aregister = strtok(arguments, ",;")) == NULL || (avalue = strtok(NULL, " ;")) == NULL) {
            printf("Error: syntax error in line #%d (unknown argument)\n", lineno);
            return -2;
          }
          bits = register_bits(aregister);
          if (bits == UNKNOWN_REGISTER) {
            printf("Error: unknown register \"%s\" in line #%d\n", aregister, lineno);
            fclose(in);
            return -2;
          }
          if (*avalue == '#') {
            mode = DIRECT;
            avalue++;
          } else
            mode = RELATIVE;
          c = *avalue;
          if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
            //
            // value is label (patch later)
            //
            strcpy(reference[referenceno].label, avalue);
            reference[referenceno].value = address + 1;
            reference[referenceno].lineno = lineno;
            reference[referenceno].mode = mode;
            referenceno++;
            value = msb = lsb = 0;
            strcpy(valuestr, avalue);
            strcpy(operand, label_value(avalue, labelno, mode, address));
          } else {
            value = operand_value(avalue);
            sprintf(valuestr, "$%04X", value);
            msb = (value & 0xFF00) >> 8;
            lsb = value & 0x00FF;
            sprintf(operand, "%04X", value);
          }
          code = (code | mode | bits);
          printf("%04d: %04X %02X %-12s %-12s     ", lineno, address, code, operand, mnemonic);
          printf("%s,%s%s\n", register_name[bits], ((mode == DIRECT) ? "#" : ""), valuestr);
          mm[address++] = code;
          mm[address++] = msb;
          mm[address++] = lsb;
          break;
        case 3: // GROUP 3 (16 bit operand, irregular)
          //
          // special instructions (generic part)
          //
          if ((aregister = strtok(arguments, ",;")) == NULL) {
            printf("Error: syntax error in line #%d (unknown argument)\n", lineno);
            fclose(in);
            return -2;
          }
          if ((avalue = strtok(NULL, " ;")) == NULL) {
            avalue = aregister;
            aregister = NULL;
          }
          if (*avalue == '#') {
            mode = DIRECT;
            avalue++;
          } else
            mode = RELATIVE;
          c = *avalue;
          if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
            strcpy(reference[referenceno].label, avalue);
            reference[referenceno].value = address + 1;
            reference[referenceno].lineno = lineno;
            reference[referenceno].mode = mode;
            referenceno++;
            value = msb = lsb = 0;
            strcpy(valuestr, avalue);
            strcpy(operand, label_value(avalue, labelno, mode, address));
          } else {
            value = operand_value(avalue);
            sprintf(valuestr, "$%04X", value);
            msb = (value & 0xFF00) >> 8;
            lsb = value & 0x00FF;
            sprintf(operand, "%04X", value);
          }

          //
          // special instructions (the instructions)
          //
          condition = bits = 0xFF;
          switch (code) {
            case JUMP:
              condition = condition_bits(aregister);
              if (condition == UNKNOWN_CONDITION) {
                printf("Error: unknown condition \"%s\" in line #%d\n", aregister, lineno);
                fclose(in);
                return -2;
              }
              code = (code | mode | condition);
              break;
            case CALL:
              code = (code | mode);
              break;
            case MOVE:
              bits = register_bits(aregister);
              if (bits == UNKNOWN_REGISTER) {
                printf("Error: unknown register \"%s\" in line #%d\n", aregister, lineno);
                fclose(in);
                return -2;
              }
              code = (code | mode | bits);
              break;
            case SYSCALL:
              if (mode == RELATIVE) {
                printf("Error: syntax error in line #%d (only direct mode is allowed)\n", lineno);
                fclose(in);
                return -2;
              }
              code = (code | DIRECT);
              break;
          }

          //
          // list instruction
          //
          printf("%04d: %04X %02X %-12s %-12s     ", lineno, address, code, operand, mnemonic);
          if (condition != 0xFF)
            printf("%s,", condition_name[condition]);
          if (bits != 0xFF)
            printf("%s,", register_name[bits]);
          printf("%s%s\n", ((mode == DIRECT) ? "#" : ""), valuestr);

          //
          // generate code
          //
          mm[address++] = code;
          mm[address++] = msb;
          mm[address++] = lsb;
          break;
        default:
          printf("Error: no known group #%d for instruction '%s' in line #%d (abort)\n", group, instruction, lineno);
          fclose(in);
          return -2;
      }
    }
  }
  fclose(in);

  //
  // Fixup & resolve references
  //

  //
  // check if references are defined
  //
  for (n = 0; n < referenceno; n++) {
    found = false;
    for (i = 0; i < labelno; i++) {
      if (strcasecmp(symbols[i].label, reference[n].label) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      printf("Error: undefined reference in line %d: '%s'\n", reference[n].lineno, reference[n].label);
      errors++;
    }
  }

  //
  // check if all labels are used
  //
  for (n = 0; n < labelno; n++) {
    if (symbols[n].mode == PREDEFINED)
      continue;
    found = false;
    for (i = 0; i < referenceno; i++) {
      if (strcasecmp(reference[i].label, symbols[n].label) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      printf("Warning: unused label in line %d: '%s'\n", symbols[n].lineno, symbols[n].label);
      warnings++;
    }
  }
  printf("%d lines read (%d error%s, %d warning%s)\n", lineno, errors, ((errors == 1) ? "" : "s"), warnings, ((warnings == 1) ? "" : "s"));
  if (errors > 0)
    return -4;

  //
  // resolve references (in-memory)
  //
  for (n = 0; n < referenceno; n++) {
    for (i = 0; i < labelno; i++) {
      if (strcasecmp(symbols[i].label, reference[n].label) == 0)
        break;
    }
    mem = reference[n].value;
    if (reference[n].mode == DIRECT) {
      mm[mem++] = (symbols[i].value & 0xFF00) >> 8;
      mm[mem++] = symbols[i].value & 0x00FF;
    } else if (reference[n].mode == RELATIVE) {
      value = (symbols[i].value - reference[n].value - 2);
      mm[mem++] = (value & 0xFF00) >> 8;
      mm[mem++] = value & 0x00FF;
    } else {
      printf("Error: unknown or unimplemented addressing mode\n");
      return -3;
    }
  }

  //
  // report
  //
  printf("\nSYMBOL TABLE\n");
  for (n = 0; n < labelno; n++) {
    if (symbols[n].mode == PREDEFINED)
      continue;
    printf("%-12s 0x%04X\n", symbols[n].label, symbols[n].value);
  }

  printf("\nREFERENCE TABLE\n");
  for (n = 0; n < referenceno; n++) {
    switch (reference[n].mode) {
      case DIRECT:
        s = (char *) "direct";
        break;
      case RELATIVE:
        s = (char *) "relative";
        break;
      case DEFINED:
        s = (char *) "defined";
        break;
      default:
        s = (char *) "undefined";
    }
    printf("0x%04X  %-12s  %-12s  (line %d)\n", reference[n].value, reference[n].label, s, reference[n].lineno);
  }

  printf("\nMEMORY DUMP\n");
  for (n = 0; n < address; n += CHUNK) {
    if ((n + CHUNK) < address)
      l = CHUNK;
    else
      l = address - n;
    printf("%04X ", n);
    for (m = n; m < (n + l); m++)
      printf(" %02X", mm[m]);

    for (m = (n + l); m < (n + CHUNK); m++)
      printf(" __");

    printf("  ");

    for (m = n; m < (n + l); m++)
      printf("%c", translate[mm[m] & 0xFF]);
    for (m = (n + l); m < (n + CHUNK); m++)
      printf("_");
  
    printf("\n");
  }

  end = address - 1;
  printf("\nADDRESSES\n");
  printf("code low  : %04x\n", start);
  printf("code high : %04x\n", end); 
  printf("execution : %04x\n", transfer);

  //
  // output hex file
  //
  sprintf(sstart, "%04X", start);
  sprintf(send, "%04X", end);
  sprintf(stransfer, "%04X", transfer);

  sargv[0] = (char *) "save";
  sargv[1] = sstart;
  sargv[2] = send;
  sargv[3] = stransfer;
  sargv[4] = output;
  bytes = s19save(5, sargv);
  printf("\n%d bytes written\n", bytes);


  return 0;
}

//
// include - include (definitions only: use asmacs to include assembler and/or define macros)
//
int include(char *filename)
{
  int errors = 0, lineno = 0;
  char *label, *directive, *argument, *p, c, asmline[READ_BUFFER + 1];;
  FILE *in;
  uint16_t value;

  if ((in = fopen(filename, "r")) == NULL) {
    printf("Error: cannot open '%s' for reading\n", filename);
    return -1;
  }
  while (getLine(asmline, READ_BUFFER, in, false) != EOF) {
    lineno++;
    c = asmline[0];
    p = &asmline[0];

    //
    // skip lines without commands
    //
    if ((c == ';') || (strlen(asmline) == 0))
      continue;

    //
    // must have 3 fields: <label> .define #<argument>
    //
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
      label = strtok(p, " ;");
      directive = strtok(NULL, " ;");
      if (strcasecmp(directive, ".define") != 0) {
        printf("Error: error in line #%d of '%s' (expected: .define)\n", lineno, filename);
        errors++;
        continue;
      }
      argument =  strtok(NULL, " ;");
      if (*argument != '#') {
        printf("Error: error in line #%d of '%s' (only immedate values are allowed)\n", lineno, filename);
        errors++;
        continue;
      }
      value = operand_value(++argument);

      //
      // add to symbol table
      //
      strcpy(symbols[labelno].label, label);
      symbols[labelno].value = value;
      symbols[labelno].lineno = lineno;
      symbols[labelno].mode = PREDEFINED;
      labelno++;
    } else {
      printf("Error: error in line #%d of '%s' (expected: label definition)\n", lineno, filename);
      errors++;
    }
  }
  fclose(in);
  return errors;
}

//
// find label value (no forward references;-)
//
char *label_value(char *label, int max, int mode, uint16_t address)
{
  static char value[5];
  int n;
  bool found;

  found = false;
  for (n = 0; n < max; n++) {
    if (strcasecmp(symbols[n].label, label) == 0) {
      found = true;
      break;
    }
  }
  if (found) {
    if (mode == DIRECT)
      sprintf(value, "%04X", symbols[n].value);
    else
      sprintf(value, "%04X", (symbols[n].value - address - 3) & 0xFFFF);
  } else
    strcpy(value, UNDEFINED);
  return value;
}

