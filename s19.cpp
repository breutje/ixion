//
// s19 Motorola HEX format utilities
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "memory.h"
#include "s19.h"

//
// s19load S1/S9
//
int s19load(int argc, char *argv[])
{
  FILE *fp;
  int reply, bytes = 0, lineno = 0;
  size_t n = 0;
  char *sline = NULL;

  if (argc != 2) {
    printf("Error: Usage: load <filename>\n");
    return -1;
  }

  //
  // load hex file
  //
  if ((fp = fopen(argv[1], "r")) == NULL) {
    printf("Error: cannot open '%s' for reading\n", argv[1]);
    return -1;
  }
  while (getline(&sline, &n, fp) != (ssize_t) -1) {
    lineno++;
    reply = s19parse(mm, MEMSIZE, sline);
    if (reply < 0) {
      printf("Error: error reading '%s' in line %d\n", argv[1], lineno);
      break;
    }
    bytes += reply;
  }
  free(sline);
  fclose(fp);
  printf("%d bytes read\n", bytes);
  return bytes;
}

//
// save S1/S9
//
// now:     save <start> <end> <filename>
//
//          save <start> <end> <transfer> <filename>
//          label is set to filename.
//
int s19save(int argc, char *argv[])
{
  uint16_t start = 0, transfer = 0, end = MEMSIZE - 1;
  uint16_t n, m, length, checksum;
  char *s, *endptr;
  FILE *fp;

  if (argc != 5) {
    printf("Error: Usage: save <start> <end> <transfer> <filename>\n");
    return 0;
  }
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
  transfer = (uint16_t) strtol(argv[3], &endptr, 16);
  if (*endptr != '\0') {
    printf("Error: '%s' is not a hexadecimal number\n", argv[2]);
    return 1;
  }
  if (transfer < 0 || transfer > MEMSIZE) {
    printf("Error: '%04X' is out of range (0000 - %04X)\n", end, MEMSIZE);
    return 2;
  }
  if (transfer < start || transfer > end) {
    printf("Error: transfer address '%04X' is not within range (%04X - %04X)\n", transfer, start, end);
    return 2;
  }

  //
  // output hex file
  //
  if ((fp = fopen(argv[4], "w")) == NULL) {
    printf("Error: cannot open '%s' for writing\n", argv[3]);
    return 4;
  }

  //
  // S0 SREC
  //
  s = argv[4];
  length = strlen(s);
  checksum = length + 3;
  fprintf(fp, "S0%02X0000", length + 3);
  for (m = 0; m < length; m++) {
    fprintf(fp, "%02X", *s);
    checksum += *s;
    s++;
  }
  fprintf(fp, "%02X\n", ~checksum & 0xFF);

  //
  // S1 SREC
  //
  for (n = start; n <= end; n += S19_CHUNK) {
    if ((n + S19_CHUNK) <= end)
      length = S19_CHUNK;
    else
      length = end - n + 1;
    fprintf(fp, "S1%02X%04X", (length + 3), n);
    checksum = (length + 3) + ((n & 0xFF00) >> 8) + (n & 0xFF);
    for (m = n; m < (n + length); m++) {
      fprintf(fp, "%02X", mm[m]);
      checksum += mm[m];
    }
    fprintf(fp, "%02X\n", ~checksum & 0xFF);
  }

  //
  // S9 SREC
  //
  checksum = 3 + ((transfer & 0xFF00) >> 8) + (transfer & 0xFF);
  fprintf(fp, "S903%04X%02X\n", transfer, ~checksum & 0xFF);

  fclose(fp);
  return (end - start + 1);
}

//
// s19parse - parse S1 records
//
int s19parse(uint8_t memory[], int memsize, char *line) {
  char *start, *pos;
  uint16_t count, address, check, checksum, data, length, msb, lsb, byte, pass;

  //
  // skip non S1 records
  //
  if ((pos = strstr(line, "S1")) == NULL)
    return 0;

  //
  // minimal length
  //
  if (strlen(pos) < 10)
    return 0;

  //
  // get length (from S1 string)
  //
  pos += 2;
  if ((length = s19byte(pos)) < 3)
    return 0;

  //
  // address + data + checksum should be at least 'length'
  //
  pos += 2;
  if (strlen(pos) < length)
    return 0;

  //
  // get address (from S1 string)
  //
  if ((msb = s19byte(pos)) < 0)
    return 0;
  pos += 2;
  if ((lsb = s19byte(pos)) < 0)
    return 0;
  pos += 2;
  address = (msb << 8) | lsb;

  //
  // 2 pass read data: on pass 1 only check, on pass 2 (checksum is ok) store
  //
  count = 0;
  checksum = length + msb + lsb;
  start = pos;
  for (pass = 1; pass <= 2; pass++) {
    pos = start;
    address = (msb << 8) | lsb;
    for (data = 0; data < (length - 3); data++) {
      if ((byte = s19byte(pos)) < 0)
        return 0;
      checksum += byte;
      if (pass == 2) {
        mm[address] = byte;
        count++;
      }
      address++;
      pos += 2;
    }
    if ((check = s19byte(pos)) < 0)
      return 0;
    if (check != (~checksum & 0xFF))
      break;
  }
  return count;
}

//
// s19byte - string (hex) to byte
//
uint8_t s19byte(char *input) {
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

