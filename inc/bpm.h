#ifndef BPM_H
#define BPM_H
#include <stdint.h>
#include <stdbool.h>

struct data_buffer_t
{
    uint8_t   size;
    uint16_t* position;
    float*    value;
};

struct bpm_result_t
{
    float  sys;
    float  dia;
    float  pulse;
};

struct bpm_status_t
{
    float tmp_max;
    float tmp_min;
    float meas_value;
    bool  max_found;
    bool  min_found;
    uint16_t counter;
    uint16_t pos_max;
    uint16_t pos_min;
};

typedef struct data_buffer_t data_buffer_t;
typedef struct bpm_result_t  bpm_result_t;
typedef struct bpm_status_t  bpm_status_t;

void bpm_update_status(float input, float filt_output);
void bpm_init(void);
bpm_result_t bpm_get_result(void);
data_buffer_t* get_local_min_buff(void);
data_buffer_t* get_local_max_buff(void);

#endif