//
// monmain -  simple ixion monitor and cpu
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ixion.h"
#include "memory.h"
#include "instructions.h"

//
// monitor
//
int main(int argc, char *argv[])
{
  int reply;

  if (argc > 2) {
    printf("Usage: %s [ <s19> ]\n", argv[0]);
    return 1;
  }
  if (argc == 2 && access(argv[1], R_OK) != 0) {
    printf("Error: cannot access \"%s\"\n", argv[1]);
    return 2;
  }
  reply = monixion(argc, argv);
  return reply;
}

