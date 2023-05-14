#include <bpm.h>
#include <peak_filter.h>

#define BUFFSIZE 64

static float amplitudes[BUFFSIZE];
static float max_values[BUFFSIZE];
static float min_values[BUFFSIZE];
static uint16_t max_positions[BUFFSIZE];
static uint16_t min_positions[BUFFSIZE];

void bpm_init(data_buffer_t* local_min, data_buffer_t* local_max)
{
    local_min->size = 0;
    local_min->position = min_positions;
    local_min->value = min_values;
    local_max->size = 0;
    local_max->position = max_positions;
    local_max->value = max_values;
}