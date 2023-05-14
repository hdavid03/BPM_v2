#include <peak_filter.h>
#include <stdint.h>

static float A_COEFFS[] = {-1.998072905031434, 0.998112346058487};
static float B_COEFFS[] = {9.438269707564562e-04, 0, -9.438269707564562e-04};
static uint8_t TAP_NUM  = 3;

static void update_filter_status(peak_filter_t*, float, float);

void peak_filter_init(peak_filter_t* filter)
{
    filter->a = A_COEFFS;
    filter->len_a = TAP_NUM - 1;
    filter->b = B_COEFFS;
    filter->len_b = TAP_NUM;
}

static void update_filter_status(peak_filter_t* filter,
                                 float input_sample, float pre_result)
{
    float tmp = 0.0;
    for (uint8_t ii = 0; ii < filter->len_a; ii++)
    {
        tmp = filter->x[ii];
        filter->x[ii] = input_sample;
        input_sample = tmp;
    }
    for (uint8_t ii = 0; ii < filter->len_b; ii++)
    {
        tmp = filter->y[ii];
        filter->y[ii] = pre_result;
        pre_result = tmp;
    }
}

float filter_sample(peak_filter_t* filter, float input_sample, float pre_result)
{
    float tmp_a = 0.0;
    float tmp_b = 0.0;
    update_filter_status(filter, input_sample, pre_result);
    for (uint8_t ii = 0; ii < filter->len_a; ii++)
    {
        tmp_a += filter->a[ii] * filter->y[ii];
    }
    for (uint8_t ii = 0; ii < filter->len_b; ii++)
    {
        tmp_b += filter->b[ii] * filter->x[ii];
    }
    return tmp_b - tmp_a;
}