//
// monitor - simple monitor for ixion
//
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <string.h>

#include "s19.h"
#include "ixion.h"
#include "memory.h"
#include "instructions.h"

#define MAX_ARGUMENTS       10
#define CHUNK               16 
#define PROMPT        "IXION>"  // "C:\\>" ;-)

//
// vt100 escapes
//
#define INVERSE       "\033[7m"
#define NORMAL        "\033[m"

//
// prototypes
//
int monitor(void);
int getLine(char line[], int maxlen, FILE *stream, bool history);
int s_parse(uint8_t memory[], int memsize, char *line);
int hexbyte(char *input);
char *bits(uint8_t);
char *byte(uint8_t);
char *bytes(uint16_t);
char *verbose(char * input);
int required(const char *name, const char *command);
int match(const char *name, const char *command);

//
// monitor commands
//
int debugmode(int argc, char *argv[]);
int dump(int argc, char *argv[]);
int fill(int argc, char *argv[]);
int help(int argc, char *argv[]);
int list(int argc, char *argv[]);
int quit(int argc, char *argv[]);
int registers(int argc, char *argv[]);
int run(int argc, char *argv[]);
int setbyte(int argc, char *argv[]);
int version(int argc, char *argv[]);

//
// command structure
//
struct _commands {
  int (*function)(int argc, char *argv[]);
  bool show;
  const char *name;
  const char *arguments;
  const char *description;
} commands[] = {
  { debugmode, true,  "DEbug", "                                   ", "toggle debug mode" },
  { dump,      true,  "Dump", "[<start> <end>]                     ", "hexadecimal dump of memory" },
  { fill,      true,  "Fill", "[<byte>|<start> <end> [<byte>]]     ", "fill memory" },
  { help,      true,  "Help", "                                    ", "provide help on specific keywords" },
  { list,      true,  "List", "[<start> <end>]                     ", "disassemble memory" },
  { s19load,   true,  "LOad", "<filename>                          ", "load file" },
  { quit,      true,  "Quit", "                                    ", "quit monitor" },
  { registers, true,  "Registers", "                               ", "display registers" },
  { run,       true,  "RUn", " [<start>[<end>]]                    ", "run from <start> (or 0x0000) until <end> or HALT" },
  { s19save,   true,  "SAve", "<start> <end> <transfer> <filename> ", "save memory dump to file" },
  { setbyte,   true,  "Set", "<address> <byte>                     ", "set memory byte" },
  { version,   true,  "Version", "                                 ", "show monitor version" },
  { NULL }
};

//
// version and build
//
extern char *_version;
extern int _build;
extern char *_date;

//
// globals
//
uint16_t transfer;

//
// use monitor ineractively or just to run ixion
//
int monixion(int argc, char *argv[])
{
  int result;

  version(0, 0);

  if (argc == 1)
    result = monitor();
  else if (argc == 2) {
    transfer = s19load(argc, argv);
    disassemble = false;
    execute = true;
    //ixion(0, top); // set disassemble to 0 for production!
    result = monitor();
  } else {
    printf("Usage: %s [ <program> ]\n", argv[0]);
    result = 1;
  }
  return result;
}

//
// simple monitor: help, run, load, list, dump, etc.
//
int monitor(void) {
  char *cmd, command[READ_BUFFER + 1];
  int n, argc, result, running = 1;
  char *found, *argv[MAX_ARGUMENTS];
  
  while (running) {
    argc = 0;
    printf(PROMPT);
    result = getLine(command, READ_BUFFER, stdin, true);
    putchar('\n');
    if (result == 0 || result == EOF)
      continue;
    if (debug)
      printf("#%2d [%s]\n", (int) strlen(command), verbose(command));
    cmd = strtok(command, " ,");
    do {
      argv[argc] = cmd;
      argc++;
    } while ((cmd = strtok(NULL, " ,")) != NULL && argc < MAX_ARGUMENTS);
    if (debug)
      printf("DONE (#%d) command = [%s]\n", argc, argv[0]);

    //
    // find and execute the command
    //
    for (n = 0; (found = (char *) commands[n].name) != NULL; n++) {
      if (required(commands[n].name, argv[0]) || match(commands[n].name, argv[0])) {
        if (commands[n].function(argc, argv) == -1)
          running = 0;
        break;
      }
    }
    if (found == NULL)
      printf("Error: '%s' not found\n", argv[0]);
  }
  return 0;
}

//
// help
//
int help(int argc, char *argv[]) {
  int n;

  printf("Help:\n\n");
  for (n = 0; commands[n].name != NULL; n++) {
    if (commands[n].show)
      printf("  %s %s     ; %s\n", (char *) commands[n].name, (char *) commands[n].arguments, (char *) commands[n].description);
  }
  printf("\n");
  return 0;
}

//
// dump
//
int dump(int argc, char *argv[]) {
  uint16_t start = 0, end = MEMSIZE - 1;
  uint16_t n, m, length;
  char *endptr;

  if (argc == 3) {
    start = (uint16_t) strtol(argv[1], &endptr, 16);
    if (*endptr != '\0') {
      printf("Error: '%s' is not a hexadecimal number\n", argv[1]);
      return 1;
    }
    if (start < 0 || start > MEMSIZE) {
      printf("Error: '%04X' is out of range (0000 - %04X)\n", start, MEMSIZE);
      return 2;
    }
    end =  (uint16_t) strtol(argv[2], &endptr, 16);
    if (*endptr != '\0') {
      printf("Error: '%s' is not a hexadecimal number\n", argv[2]);
      return 1;
    }
    if (end < 0 || end > MEMSIZE) {
      printf("Error: '%04X' is out of range (0000 - %04X)\n", end, MEMSIZE);
      return 2;
    }
    if (end < start) {
      printf("Error: end (%04X) is before start (%04X)\n", end, start);
      return 3;
    }
  }

  //
  // output
  //
  for (n = start; n <= end; n += CHUNK) {
    if ((n + CHUNK) <= end)
      length = CHUNK;
    else
      length = end - n + 1;
    printf("%04X ", n);
    for (m = n; m < (n + length); m++)
      printf(" %02X", mm[m]);
    printf("\n");
  }
  return 0;
}

//
// run [ <start> [ <end> ]]
//
int run(int argc, char *argv[]) {
  uint16_t start = 0, end = MEMSIZE - 1;
  char *endptr;

  if (argc > 3) {
    printf("Error: Usage: run [ <start> [ <end> ]]\n");
    return 1;
  }
  if (argc > 2) {
    end = (uint16_t) strtol(argv[2], &endptr, 16);
    if (*endptr != '\0') {
      printf("Error: '%s' is not a hexadecimal number\n", argv[2]);
      return 1;
    }
    if (end < 0 || end > MEMSIZE) {
      printf("Error: '%04X' is out of range (0000 - %04X)\n", end, MEMSIZE);
      return 2;
    }
  }
  if (argc > 1) {
    start = (uint16_t) strtol(argv[1], &endptr, 16);
    if (*endptr != '\0') {
      printf("Error: '%s' is not a hexadecimal number\n", argv[1]);
      return 1;
    }
    if (start < 0 || start > MEMSIZE) {
      printf("Error: '%04X' is out of range (0000 - %04X)\n", start, MEMSIZE);
      return 2;
    }
  }
  if (end < start) {
    printf("Error: end (%04X) is before start (%04X)\n", end, start);
    return 3;
  }

  printf("run from %04X - %04X\n", start, end);
  disassemble = true; // set false for production
  execute = true;
  ixion(start, end);
  printf("halt\n");
  return 0;
}

//
// list [ <start> [ <end> ]]
//
int list(int argc, char *argv[]) {
  uint16_t start = 0, end = MEMSIZE - 1;
  char *endptr;

  if (argc > 3) {
    printf("Error: Usage: list [ <start> [ <end> ]]\n");
    return 1;
  }
  if (argc > 2) {
    end =  (uint16_t) strtol(argv[2], &endptr, 16);
    if (*endptr != '\0') {
      printf("Error: '%s' is not a hexadecimal number\n", argv[2]);
      return 1;
    }
    if (end < 0 || end > MEMSIZE) {
      printf("Error: '%04X' is out of range (0000 - %04X)\n", end, MEMSIZE);
      return 2;
    }
  }
  if (argc > 1) {
    start = (uint16_t) strtol(argv[1], &endptr, 16);
    if (*endptr != '\0') {
      printf("Error: '%s' is not a hexadecimal number\n", argv[1]);
      return 1;
    }
    if (start < 0 || start > MEMSIZE) {
      printf("Error: '%04X' is out of range (0000 - %04X)\n", start, MEMSIZE);
      return 2;
    }
  }
  if (end < start) {
    printf("Error: end (%04X) is before start (%04X)\n", end, start);
    return 3;
  }

  printf("list %04X - %04X\n", start, end);
  disassemble = true;
  execute = false;
  ixion(start, end);
  printf("halt\n");

  return 0;
}

//
// setbyte <address> <byte>
//
int setbyte(int argc, char *argv[]) {
  uint16_t byte, location;
  char *endptr;

  location = (uint16_t) strtol(argv[1], &endptr, 16);
  if (*endptr != '\0') {
    printf("Error: '%s' is not a hexadecimal number\n", argv[1]);
    return 1;
  }
  if (location < 0 || location > MEMSIZE) {
    printf("Error: '%04X' is out of range (0000 - %04X)\n", location, MEMSIZE);
    return 2;
  }
  byte = (uint16_t) strtol(argv[2], &endptr, 16);
  if (*endptr != '\0') {
    printf("Error: '%s' is not a hexadecimal number\n", argv[3]);
    return 1;
  }
  if (byte < 0 || byte > 0xFF) {
    printf("Error: '%02X' is out of range (00 - FF)\n", byte);
    return 2;
  }
  mm[location] = byte;
  return 0;
}

//
// fill [ <byte> | [ <start> <end> [ <byte> ]]]
//
// 1 fill
// 2 fill ff
// 3 fill 0000 000ff
// 4 fill 0000 000ff 55
//
int fill(int argc, char *argv[]) {
  uint16_t byte, n, start = 0, end = MEMSIZE - 1;
  char *endptr;

  switch (argc) {
    case 1: 
      memset(mm, 0, MEMSIZE);
      break;
    case 2:
      if (strcmp(argv[1], "?") == 0) {
        srand(time(0));
        for (n = 0; n < MEMSIZE; n++)
          mm[n] = rand() % 0xFF;
        return 0;
      }
      byte = (uint16_t) strtol(argv[1], &endptr, 16);
      if (*endptr != '\0') {
        printf("Error: '%s' is not a hexadecimal number\n", argv[1]);
        return 1;
      }
      if (byte < 0 || byte > 0xFF) {
        printf("Error: '%02X' is out of range (00 - FF)\n", byte);
        return 2;
      }
      memset(mm, byte, MEMSIZE);
      break;
    case 3: /*FALLTHROUGH*/
    case 4:
      start = (uint16_t) strtol(argv[1], &endptr, 16);
      if (*endptr != '\0') {
        printf("Error: '%s' is not a hexadecimal number\n", argv[1]);
        return 1;
      }
      if (start < 0 || start > MEMSIZE) {
        printf("Error: '%04X' is out of range (0000 - %04X)\n", start, MEMSIZE);
        return 2;
      }
      end = (uint16_t) strtol(argv[2], &endptr, 16);
      if (*endptr != '\0') {
        printf("Error: '%s' is not a hexadecimal number\n", argv[2]);
        return 1;
      }
      if (end < 0 || end > MEMSIZE) {
        printf("Error: '%04X' is out of range (0000 - %04X)\n", end, MEMSIZE);
        return 2;
      }
      if (end < start) {
        printf("Error: end (%04X) is before start (%04X)\n", end, start);
        return 3;
      }
      if (argc == 4) {
        byte = (uint16_t) strtol(argv[3], &endptr, 16);
        if (*endptr != '\0') {
          printf("Error: '%s' is not a hexadecimal number\n", argv[3]);
          return 1;
        }
        if (byte < 0 || byte > 0xFF) {
          printf("Error: '%02X' is out of range (00 - FF)\n", byte);
          return 2;
        }
      }
      memset(&mm[start], byte, (end - start + 1));
      break;
    default:
      printf("Error: Usage: fill [ <byte> | <start> <end> [ <byte> ]]\n");
      return 1;
  }
  return 0;
}

//
// quit
//
int quit(int argc, char *argv[]) {
  return -1;
}

//
// version
//
int version(int argc, char *argv[]) {
  printf("%s.%d %s\n", _version, _build, _date);
  return 0;
}

//
// debug
//
int debugmode(int argc, char *argv[]) {
  if (debug) {
    debug = false;
    printf("debug mode is off\n");
  } else {
    debug = true;
    printf("debug mode is on\n");
  }
  return 0;
}

//
// registers
//
int registers(int argc, char *argv[]) {
  printf("PC %04X\n", pc);
  printf("SP %04X\n", sp);
  printf("R0 %04X\n", r0);
  printf("R1 %04X\n", r1);
  printf("R2 %04X\n", r2);
  printf("R3 %04X\n", r3);
  printf("CC   %02X (%s)\n", cc, bits(cc));
  return 0;
}
