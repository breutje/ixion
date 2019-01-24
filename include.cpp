//
// include
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "ixion.h"
#define READ_BUFFER 1024
int getLine(char line[], int maxlen, FILE *stream, bool history);
uint16_t operand_value(char *input);



int include(char *filename);

//
// include
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
  reply = include(argv[1]);

  return reply;
}

//
// include - include (definitions only)
//
int include(char *filename)
{
  printf("INCLUDE(%s)\n", filename);
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
      printf("DEFINE(%s) = 0x%04X\n", label, value);

    } else {
      printf("Error: error in line #%d of '%s' (expected: label definition)\n", lineno, filename);
      errors++;
    }
  }
  fclose(in);
  return errors;
}
