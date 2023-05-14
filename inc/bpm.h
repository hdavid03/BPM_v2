#ifndef BPM_H
#define BPM_H
#include <stdint.h>

struct data_buffer_t
{
    uint8_t   size;
    uint16_t* position;
    float*    value;
};

typedef struct data_buffer_t data_buffer_t;

void bpm_init(data_buffer_t*, data_buffer_t*);

#endif