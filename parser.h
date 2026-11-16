#include <stdint.h>
#include <stdbool.h>

#define P_SIZE 32

typedef struct 
{
    char line[P_SIZE];
    uint16_t index;
    uint16_t store;
    bool overflow;
    char out[64];
} Parser;

typedef struct
{
    char name[32];
    int *function;
} Operation;



void parser_feed(Parser *p, uint8_t byte);

bool parse_uint(const char *s, uint16_t *out);

static void execute_line(Parser *p);

static void led(Parser *p, char *s);

static void set(Parser *p, char *s);

static void get(Parser *p);

static void add(Parser *p, char *s);