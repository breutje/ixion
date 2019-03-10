//
// ancient utilities
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>

#include "ixion.h"

#define BUFFERSIZE               1024
#define LINE_HISTORY                5
#define ESCAPE                     27

//
// getLine - get lines ending in \n or \n from stream
//
// Bugs: vt100 escapes are hard coded.....
//
int getLine(char line[], int maxlen, FILE *stream, bool history)
{
  int c, tty, charno = 0;
  struct termios tmp, tio;
  static int lineno = 0;
  static char *lines[LINE_HISTORY + 1];

#if defined(UNIX)
  if ((tty = isatty(fileno(stream)))) {
    tcgetattr(STDIN_FILENO, &tmp);            // get current settings
    tio = tmp;                                // save for undo
    tio.c_lflag &= (~ICANON & ~ECHO);         // disable canonical mode (buffered i/o)
    tcsetattr(STDIN_FILENO, TCSANOW, &tio);
  }
#endif
  
  while (charno <= maxlen) {
    c = getc(stream);
    if (c == ESCAPE) {
      if (getc(stream) == '[') {
        switch (getc(stream)) {
          case 'A': // ESC[A   up      27 91 65 [1B 5B 41]
            if (tty)
              putchar('^');
            break;
          case 'B': // ESC[B   down    27 91 66 [1B 5B 42]
            if (tty)
              putchar('v');

//debug
            printf("lineno = %d\n", lineno);
            for (int n = 0; n < LINE_HISTORY; n++)
              printf("%d:  [%s]\n", n, lines[n] == NULL ? "(null)" : lines[n]);
//debug
            break;
#if defined(LINE_EDIT)
          case 'C': // ESC[C   right   27 91 67 [1B 5B 43]
            if (tty)
              putchar('>');
            break;
          case 'D': // ESC[D   left    27 91 68 [1B 5B 44]
            if (tty)
              putchar('<');
            break;
#endif
          default:
            break;
        }
      }
      continue;
    }
    if (c == '\t')
      c = ' ';
    if (c == EOF)
      break;
    if (c == '\n' || c == '\r')
      break;
    if (c == 0x08 || c == 0x7F) {
      if (charno > 0) {
        charno--;
        line[charno] = 0;
        if (tty)
          putchar(c);
      }
      continue;
    }
    line[charno] = c;
    if (tty)
      putchar(c);
    charno++;
  }

  //
  // store line for later use, free lines that cannot be used anymore
  //
  if (history) {
    if (charno > 0) {
      if (lines[lineno] != NULL)
      free(lines[lineno]);
      lines[lineno] = strdup(line);
      lineno++;
      if (lineno > LINE_HISTORY)
        lineno = 0;
    }
  }

  //
  // EOF (from file)
  //
  if (c == EOF)
    charno = EOF; // return end-of-file (kludge)

  line[charno] = '\0';
#if defined(UNIX)
  if (tty)
    tcsetattr(STDIN_FILENO,TCSANOW,&tmp);     // undo
#endif
  return charno;
}

//
// bits to string
//
char *bits(uint8_t byte) {
  static char output[9];

  uint8_t mask = 0x80;
  for (int n = 0; n < 8; n++) {
    output[n] = (byte & mask) == 0 ? '0' : '1';
    mask >>= 1;
  }
  output[8] = 0;
  return output;
}

//
// hexbyte - string (hex) to byte
//
int hexbyte(char *input) {
  uint8_t result, msn, lsn;
  static char *hex = (char *) "0123456789ABCDEF";
  char *b;

  if ((b = strchr(hex, *input)) == NULL)
    return -1;
  msn = (int) (b - hex);
  input++;
  if ((b = strchr(hex, *input)) == NULL)
    return -2;
  lsn = (int) (b - hex);
  result = (msn << 4) | lsn;
  return result;
}

//
// operand_value with error messages
//
uint16_t operand_value(char *input)
{
  uint16_t result;
  char *endptr;

  if (*input == '$')
    *input = '0';
  result = (uint16_t) strtol(input, &endptr, 16);
  if (*endptr != '\0') {
    printf("Error: '%s' is not a hexadecimal number\n", input);
    return 0;
  }
  return result;
}

//
// return register bits from register name
//
uint8_t register_bits(char *register_name)
{
  uint8_t bits;

  if (strcasecmp(register_name, "R0") == 0)
    bits = R0;
  else if (strcasecmp(register_name, "R1") == 0)
    bits = R1;
  else if (strcasecmp(register_name, "R2") == 0)
    bits = R2;
  else if (strcasecmp(register_name, "R3") == 0)
    bits = R3;
  else
    bits = UNKNOWN_REGISTER;
  return bits;
}

//
// return condition bits from condition name
//
uint8_t condition_bits(char *condition_name)
{
  uint8_t bits;

  if (strcasecmp(condition_name, "NZ") == 0)
    bits = NONZERO;
  else if (strcasecmp(condition_name, "Z") == 0)
    bits = ZERO;
  else if (strcasecmp(condition_name, "NC") == 0)
    bits = NOCARRY;
  else if (strcasecmp(condition_name, "C") == 0)
    bits = CARRY;
  else
    bits = UNKNOWN_CONDITION;
  return bits;
}

//
// verbose - print non-printable characters verbose
//
char *verbose(char * input) {
  static char output[BUFFERSIZE];
  char c[6];

  output[0] = 0;
  for (size_t n = 0; n < strlen(input); n++) {
    if (input[n] < ' ' || input[n] > '~')
      sprintf(c, "%%%02X", input[n]);
    else
      sprintf(c, "%c", input[n]);
   strcat(output, c);
  }
  return output;
}

//
// true if command matches required (uppercase) part
//
int required(const char *name, const char *command)
{
  while (isupper(*name)) {
    if (*name != toupper(*command))
      return 0;
    name++;
    command++;
  }
  return (*command == '\0');
}

//
// true if command matches required + optional (case insensitive)
//
int match(const char *name, const char *command)
{
  while (*name) {
    if (toupper(*name) != toupper(*command))
      return 0;
    name++;
    command++;
  }
  return (*name == *command);
}

