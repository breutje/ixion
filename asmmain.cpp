//
// asmain - assemble ixion to memory and S1/S9 file
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ixion.h"

//
// assembler
//
int main(int argc, char *argv[])
{
  int reply;

  if (argc != 2) {
    printf("Usage: %s <source>\n", argv[0]);
    return 1;
  }
  if (access(argv[1], R_OK) != 0) {
    printf("Error: cannot access \"%s\"\n", argv[1]);
    return 2;
  }
  reply = asmixion(2, argv);

  return reply;
}

