//
// genixion - generate instructions.h
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ixion.h"

//
// prototypes, etc.
//
char *bits(uint8_t);

//
// main
//
int main(void)
{
  FILE *out;
  int n, group, previous = 0;
  size_t i;
  time_t ticks;
  uint8_t code;
  char buffer[42];
  struct tm* tm_info;

  if ((out = fopen("instructions.h", "w")) == NULL) {
    printf("Error: cannot open 'instructions.h' for writing\n");
    return -1;
  }

  time(&ticks);
  tm_info = localtime(&ticks);
  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

  fprintf(out, "//\n");
  fprintf(out, "// %s.%d cpu definitions (generated at %s) DO NOT EDIT\n", _version, _build, buffer);
  fprintf(out, "//\n\n");

  for (n = 0; instructions[n].mnemonic != NULL; n++) {
    strcpy(buffer, instructions[n].mnemonic);
    for (i = 0; i < strlen(buffer); i++)
      buffer[i] = toupper(buffer[i]);
    code = instructions[n].code;
    group = (code & 0xc0) >> 6;
    if (group != previous) {
      previous = group;
      fprintf(out, "\n");
    }
    fprintf(out, "#define %-12s 0x%02X  // %d: 0b%s\n", buffer, code, group, bits(code));
  }

  fprintf(out, "\n");
  fclose(out);
  printf("%d instructions defined and saved to \"instructions.h\"\n", n);
  return 0;
}
