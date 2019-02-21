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
int main(int argc, char *argv[]) {
  int reply;

  if (argc != 2) {
    printf("Error: Usage: disixion <objectfile>\n");
    return -1;
  }
  reply = disixion(argc, argv);
  return reply;
}

