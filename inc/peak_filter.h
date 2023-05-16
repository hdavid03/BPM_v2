#ifndef PEAK_FILTER_H
#define PEAK_FILTER_H
#include <stdint.h>

struct peak_filter_t
{
    float* a;
    float* b;
    float* y;
    float* x;
    float pre_result;
    uint8_t len_a;
    uint8_t len_b;   
};

typedef struct peak_filter_t peak_filter_t;

float filter_sample(peak_filter_t*, float input);
void peak_filter_init(peak_filter_t*);

#endif