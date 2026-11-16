#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>
#include <stdint.h>

#define RB_SIZE 512
#define MASK (RB_SIZE - 1)

typedef struct{
    _Atomic uint16_t head;
    _Atomic uint16_t tail;
    uint8_t buffer[RB_SIZE];
} RingBuffer;

bool rb_is_empty(const RingBuffer *rb);
bool rb_is_full(const RingBuffer *rb);
bool rb_pop(RingBuffer *rb, uint8_t *value);
bool rb_push(RingBuffer *rb, uint8_t value);
void rb_init(RingBuffer *rb);

#endif