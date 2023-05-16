#include <bpm.h>
#include <peak_filter.h>
#include <math.h>

#define BUFFSIZE    64U
#define TMP_DEFAULT 999.9f
#define FS          960.0f
#define SYS         0.7f
#define DIA         0.65f

static float    amplitudes[BUFFSIZE];
static float    max_values[BUFFSIZE];
static float    min_values[BUFFSIZE];
static float    meas_values[BUFFSIZE];
static uint16_t meas_positions[BUFFSIZE];
static uint16_t max_positions[BUFFSIZE];
static uint16_t min_positions[BUFFSIZE];

static data_buffer_t local_min_buff;
static data_buffer_t local_max_buff;
static data_buffer_t meas_buff;
static bpm_status_t status;

static void put_data(data_buffer_t*, float, uint16_t);
static uint8_t calculate_amplitudes(void);
static float calculate_systolic(float, uint8_t);
static float calculate_diastolic(float, uint8_t, uint8_t);
static float calculate_pulse(void);

data_buffer_t* get_local_max_buff(void)
{
    return &local_max_buff;
}

data_buffer_t* get_local_min_buff(void)
{
    return &local_min_buff;
}

static float calculate_diastolic(float exp_dia, uint8_t map_index, uint8_t len)
{
    float res;
    float delta = 1.0f;
    float tmp;
    for (uint8_t ii = map_index + 1; ii < len; ii++)
    {
        tmp = fabsf(amplitudes[ii] - exp_dia);
        if (tmp < delta)
        {
            delta = tmp;
            res = meas_buff.value[ii];
        }
    }
    return res;
}

static float calculate_systolic(float exp_sys, uint8_t map_index)
{
    float res;
    float delta = 1.0f;
    float tmp;
    for (uint8_t ii = 0; ii < map_index; ii++)
    {
        tmp = fabsf(amplitudes[ii] - exp_sys);
        if (tmp < delta)
        {
            delta = tmp;
            res = meas_buff.value[ii];
        }
    }
    return res;
}

static float calculate_pulse(void)
{
    uint8_t n = local_max_buff.size / 3;
    float sum = 0.0f;
    for (uint8_t ii = 0; ii < n; ii++)
    {
        sum += local_max_buff.position[ii + 1] - local_max_buff.position[ii];
    }
    return 1.0f / (sum / n * (1.0f / FS)) * 60.0f;
}

bpm_result_t bpm_get_result(void)
{
    bpm_result_t result;
    uint8_t len_amplitudes = calculate_amplitudes();
    float map_value = -TMP_DEFAULT;
    uint8_t map_index = 0U;
    for (uint8_t ii = 3; ii < len_amplitudes; ii++)
    {
        if (amplitudes[ii] > map_value)
        {
            map_value = amplitudes[ii];
            map_index = ii;
        }
    }
    result.sys = calculate_systolic(map_value * SYS, map_index);
    result.dia = calculate_diastolic(map_value * DIA, map_index, len_amplitudes);
    result.pulse = calculate_pulse();
    return result;
}

static uint8_t calculate_amplitudes(void)
{
    uint8_t max_pos_limit = local_max_buff.size - 1U;
    uint8_t min_pos_limit = local_max_buff.size - 2U;
    uint8_t max_pos = 0U;
    uint8_t min_pos = 0U;
    uint8_t ii = 0U;
    if (local_max_buff.position[max_pos] < local_min_buff.position[min_pos])
    {
        amplitudes[ii] = local_max_buff.value[max_pos] - local_min_buff.value[min_pos];
    } else
    {
        amplitudes[ii] = local_max_buff.value[max_pos] - 
        ((local_min_buff.value[min_pos] + local_min_buff.value[min_pos]) / 2);
        min_pos++;
    }
    ii++;
    max_pos++;
    while ((max_pos < max_pos_limit) &&
           (min_pos < min_pos_limit))
    {
        amplitudes[ii] = local_max_buff.value[max_pos] - 
        ((local_min_buff.value[min_pos] + local_min_buff.value[min_pos]) / 2);
        max_pos++;
        min_pos++;
        ii++;
    }
    return ii;
}

static void put_data(data_buffer_t* buffer, float data, uint16_t pos)
{
    if (buffer->size < BUFFSIZE)
    {
        buffer->value[buffer->size] = data;
        buffer->position[buffer->size] = pos;
        buffer->size++;
    }
}

void bpm_init(void)
{
    local_max_buff.position = max_positions;
    local_min_buff.position = min_positions;
    meas_buff.position = meas_positions;
    local_max_buff.value = max_values;
    local_min_buff.value = min_values;
    meas_buff.value = meas_values;
    local_max_buff.size = 0U;
    local_min_buff.size = 0U;
    meas_buff.size = 0U;
    status.counter = 0U;
    status.max_found = false;
    status.min_found = false;
    status.tmp_max = -TMP_DEFAULT;
    status.tmp_min =  TMP_DEFAULT;
}

void bpm_update_status(float input, float filt_output)
{
    if (filt_output > status.tmp_max)
    {
        status.tmp_max = filt_output;
        status.max_found = true;
        status.meas_value = input;
        status.pos_max = status.counter;
    }
    else if ((filt_output < status.tmp_max) &&
             (status.counter - status.pos_max > 240) &&
             status.max_found)
    {
        status.max_found = false;
        put_data(&local_max_buff, status.tmp_max, status.pos_max);
        put_data(&meas_buff, status.meas_value, status.pos_max);
        status.tmp_max = filt_output;
    }
    if (filt_output < status.tmp_min)
    {
        status.tmp_min = filt_output;
        status.min_found = true;
        status.pos_min = status.counter;
    }
    else if ((filt_output > status.tmp_min) &&
             (status.counter - status.pos_min > 240) &&
             status.min_found)
    {
        status.min_found = false;
        put_data(&local_min_buff, status.tmp_min, status.pos_min);
        status.tmp_min = filt_output;
    }
    status.counter++;
}