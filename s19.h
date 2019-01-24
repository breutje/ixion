//
// s19 definitions
//
#define S19_CHUNK 32

//
// prototypes
//
int s19load(int argc, char *argv[]);
int s19save(int argc, char *argv[]);
int s19parse(uint8_t memory[], int memsize, char *line);
uint8_t s19byte(char *input);


