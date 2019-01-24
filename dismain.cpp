//
// dismain -  simple ixion disassembler of memory (and S1/S9)
//
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libgen.h>
#include <ctype.h>

#include "ixion.h"
#include "memory.h"
#include "instructions.h"

//
// disixion <file>
//
int asmixion(int argc, char *argv[]) {
  //FILE *in;

  if (argc != 2) {
    printf("Error: Usage: disixion start end\n");
    return -1;
  }
  return 0;
}

