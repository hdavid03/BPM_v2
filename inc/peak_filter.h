#ifndef PEAK_FILTER_H
#define PEAK_FILTER_H
#include <stdint.h>

struct peak_filter_t
{
    double* a;
    double* b;
    double* y;
    double* x;
    double pre_result;
    uint8_t len_a;
    uint8_t len_b;   
};

typedef struct peak_filter_t peak_filter_t;

double filter_sample(peak_filter_t*, double input);
void peak_filter_init(peak_filter_t*);

#endif