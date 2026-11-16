#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "ringbuffer.h"
#include "parser.h"

bool led_status;

void parser_feed(Parser *p, uint8_t byte){
    if(byte == '\r'){
        return;
    }
    if (p->overflow){
        if(byte == '\n'){
            p->overflow = 0;
            snprintf(p->out, sizeof p->out, "ERR TOO LONG");
            p->index = 0;
        }
        return;
    }

    if (byte == '\n'){
        p->line[p->index] = '\0';
        memset(p->out, '\0', sizeof p->out);
        execute_line(p);
        p->index = 0;
        p->overflow = 0;
        return;
    }

    if(p->index < P_SIZE - 1){
        p->line[p->index++] = byte;
    } else {
        p->overflow = 1;
    }
}

bool parse_uint(const char *s, uint16_t *out){

    if(s == NULL || s[0] == '\0'){
        return false;
    } 

    uint32_t number = 0;
    for(size_t i = 0; s[i] != '\0'; i++){

        if(s[i] < '0' || s[i] > '9'){
            return false;
        }
        number = number * 10 + (uint32_t)(s[i] - '0');
        if (number > UINT16_MAX) return false;   
    }
    *out = (uint16_t)number;
    return true;
}

static void execute_line(Parser *p){
    char *s = p->line;
    char *arg = strchr(s, ' ');
    if (arg) {
        *arg = '\0';
        arg++;
    }

    if(!strcmp(s, "LED")){
        led(p, arg);
    } else if (!strcmp(s, "SET")){
        set(p, arg);
    } else if(!strcmp(s, "GET")){
        get(p);
    } else if(!strcmp(s, "ADD")){
        add(p, arg);
    } else {
        snprintf(p->out, sizeof p->out, "ERR UNKNOWN");
    }

}

static void led(Parser *p, char *arg){
    if(arg == NULL){
        snprintf(p->out, sizeof p->out, "ERR ARGS");
        return;
    }
    if(!strcmp(arg, "ON")){
        led_status = 1;
        snprintf(p->out, sizeof p->out, "OK");
        assert(led_status == true);
    } else if (!strcmp(arg, "OFF")) {
        led_status = 0;
        snprintf(p->out, sizeof p->out, "OK");
        assert(led_status == false);
    } else{
        snprintf(p->out, sizeof p->out, "ERR ARGS");
    }

}

static void set(Parser *p, char *arg){
    if(arg == NULL){
        snprintf(p->out, sizeof p->out, "ERR ARGS");
        return;
    }
    uint16_t number;
    if(parse_uint(arg, &number)){
        if(number > 1000){
            snprintf(p->out, sizeof p->out, "ERR VALUE");
            return;
        }
        p->store = number;
        snprintf(p->out, sizeof p->out, "%u", (unsigned)p->store);
    } else{
        snprintf(p->out, sizeof p->out, "ERR VALUE");
    }
}

static void get(Parser *p){
    snprintf(p->out, sizeof p->out, "%u", (unsigned)(p->store));
}

static void add(Parser *p, char *arg){
    
    char *arg1 = arg;
    if (arg1 == NULL){
        snprintf(p->out, sizeof p->out, "ERR ARGS");
        return;
    }
    char *arg2 = strchr(arg, ' ');
    if(arg2 == NULL){
        snprintf(p->out, sizeof p->out, "ERR ARGS");
        return;
    }
    *arg2 = '\0';
    arg2++;
    
    uint16_t number1; 
    uint16_t number2;
    uint32_t result;
    if(parse_uint(arg1, &number1) && parse_uint(arg2, &number2)){
        result = number1 + number2;
        snprintf(p->out, sizeof p->out, "%u", (unsigned)result);
    } else {
        snprintf(p->out, sizeof p->out, "ERR VALUE");
    }
    
}


//static Operation operations[4] = {{"LED", led}, {"SET", set}, {"GET", get}, {"ADD", add}};
void parser_init(Parser *p){
    p->index = 0;
    p->store = 0;
    p->overflow = 0;
    p->out[0] = '\0';
}

static void feed_str(Parser *p, const char *s) {
    for (; *s; s++) parser_feed(p, (uint8_t)*s);
}

static RingBuffer rb;
static Parser p;

// static void *producer(void *arg) {
//     (void)arg;
//     const char *cmds[9] = {"LED ON\n", "LED OFF\n", "SET 1\n", "GET\n", "ADD 10 200\n", "LED OfN\n", "Lad OfN\n", "SET 124215125125\n", "ADD h 12\n"};

//     for (size_t i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
//         for (int j = 0; j < 20; ){
//             if (rb_push(&rb, cmds[i][j])) {
//                 if(cmds[i][j] == '\n') break;
//                 j++;
//             }
//         }
//     }
//     return NULL;
// }




int main(void){
    rb_init(&rb);
    parser_init(&p);
    feed_str(&p, "LED ON\n");     assert(strcmp(p.out, "OK") == 0);
    feed_str(&p, "SET 5000\n");   assert(strcmp(p.out, "ERR VALUE") == 0);
    feed_str(&p, "ADD 3 4\r\n");  assert(strcmp(p.out, "7") == 0);
    feed_str(&p, "ADD 5\n");      assert(strcmp(p.out, "ERR ARGS") == 0);
    feed_str(&p, "\n");           assert(strcmp(p.out, "ERR UNKNOWN") == 0);
    Parser parser;
    parser_init(&parser);

    // 100 Zeichen lange Zeile, danach muss er sich erholen
    char longline[101];
    memset(longline, 'A', 100);
    longline[100] = '\0';
    feed_str(&parser, longline);
    feed_str(&parser, "\n");
    assert(strcmp(parser.out, "ERR TOO LONG") == 0);
    feed_str(&parser, "GET\n");
    assert(strcmp(parser.out, "0") == 0);

    // mehrere Befehle in einem Stück
    feed_str(&parser, "SET 1\nGET\n");
    assert(strcmp(parser.out, "1") == 0);

    // Overflow bei Zahl
    feed_str(&parser, "ADD 99999 1\n");
    assert(strcmp(parser.out, "ERR VALUE") == 0);

    // Grenzfall ADD
    feed_str(&parser, "ADD 65535 65535\n");
    assert(strcmp(parser.out, "131070") == 0);

    // SET Grenzen
    feed_str(&parser, "SET 1000\n"); assert(strcmp(parser.out, "1000") == 0);
    feed_str(&parser, "SET 1001\n"); assert(strcmp(parser.out, "ERR VALUE") == 0);

    // fehlende Argumente
    feed_str(&parser, "LED\n");  assert(strcmp(parser.out, "ERR ARGS") == 0);
    feed_str(&parser, "SET\n");  assert(strcmp(parser.out, "ERR ARGS") == 0);

    // Zahl mit Müll
    feed_str(&parser, "SET 12a\n"); assert(strcmp(parser.out, "ERR VALUE") == 0);

    srand(1);
    Parser f;
    parser_init(&f);
    for (int i = 0; i < 1000000; i++) {
        parser_feed(&f, (uint8_t)(rand() & 0xFF));
    }

    return 0;
}