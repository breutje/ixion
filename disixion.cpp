//
// disixion -  simple ixion disassembler
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

//
// version and build
//
extern char *_version;
extern int _build;
extern char *_date;

//
// disixion <file>
//
int disixion(int argc, char *argv[]) {
  uint16_t start, end, transfer;

  start = 0;
  end = MEMSIZE;
  transfer = 0x1234;
  
  if (argc != 2) {
    printf("Error: Usage: disixion <filename>\n");
    return -1;
  }

  printf("%s.%d\n", _version, _build);
  transfer = s19load(argc, argv);

  printf("0x%04X - 0x%04X @0x%04X\n", start, end, transfer);
  


  return 0;
}

