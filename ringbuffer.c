#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>

#include "ringbuffer.h"

void rb_init(RingBuffer *rb){
    atomic_init(&rb->head, 0);
    atomic_init(&rb->tail, 0);
}

bool rb_push(RingBuffer *rb, uint8_t value){
    uint16_t h = atomic_load_explicit(&rb->head, memory_order_relaxed);
    uint16_t t = atomic_load_explicit(&rb->tail, memory_order_acquire);

    if ((uint16_t)(h - t) >= RB_SIZE){
        return false;
    }

    rb->buffer[h & MASK] = value;
    atomic_store_explicit(&rb->head, (uint16_t)(h+1), memory_order_release);
    return true;
}

bool rb_pop(RingBuffer *rb, uint8_t *value){
    uint16_t t = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    uint16_t h = atomic_load_explicit(&rb->head, memory_order_acquire);


    if (h == t){
        return false;
    }

    *value = rb->buffer[t & MASK];
    atomic_store_explicit(&rb->tail, (uint16_t)(t+1), memory_order_release);
    return true;
}

bool rb_is_full(const RingBuffer *rb){
    return (uint32_t)(rb->head - rb->tail) >= RB_SIZE;
    
}

bool rb_is_empty(const RingBuffer *rb){
    return (uint32_t)(rb->head - rb->tail) == 0;
}

// #include <pthread.h>

// #define N 1000000
// static RingBuffer rb;

// static void *producer(void *arg) {
//     (void)arg;
//     for (int i = 0; i < N; ) {
//         if (rb_push(&rb, (uint16_t)(i & 0x7FFF))) i++;
//     }
//     return NULL;
// }

// static void *consumer(void *arg) {
//     (void)arg;
//     for (int i = 0; i < N; ) {
//         uint8_t v;
//         if (rb_pop(&rb, &v)) {
//             assert(v == (uint16_t)(i & 0x7FFF));
//             i++;
//         }
//     }
//     return NULL;
// }

// static int main(void){
//     rb_init(&rb);
//     pthread_t thread1;
//     pthread_t thread2;
//     pthread_create(&thread1, NULL, producer, NULL);
//     pthread_create(&thread2, NULL, consumer, NULL);
// }
