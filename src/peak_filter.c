#include <peak_filter.h>
#include <stdint.h>

static float A_COEFFS[] = {-1.998072905031434F, 0.998112346058487F};
static float B_COEFFS[] = {9.438269707564562e-04F, 0.0F, -9.438269707564562e-04F};
static float X[] = {0.0F, 0.0F, 0.0F};
static float Y[] = {0.0F, 0.0F};
static uint8_t TAP_NUM  = 3;

static void update_filter_status(peak_filter_t*, float);

void peak_filter_init(peak_filter_t* filter)
{
    filter->a = A_COEFFS;
    filter->len_a = TAP_NUM - 1;
    filter->b = B_COEFFS;
    filter->len_b = TAP_NUM;
    for (int ii = 0; ii < TAP_NUM; ii++)
    {
        X[ii] = 0.0F;
    }
    for (int ii = 0; ii < TAP_NUM - 1; ii++)
    {
        Y[ii] = 0.0F;
    }
    filter->x = X;
    filter->y = Y;
    filter->pre_result = 0.0F;
}

static void update_filter_status(peak_filter_t* filter, float input_sample)
{
    float tmp = 0.0f;
    for (uint8_t ii = 0; ii < filter->len_b; ii++)
    {
        tmp = filter->x[ii];
        filter->x[ii] = input_sample;
        input_sample = tmp;
    }
    for (uint8_t ii = 0; ii < filter->len_a; ii++)
    {
        tmp = filter->y[ii];
        filter->y[ii] = filter->pre_result;
        filter->pre_result = tmp;
    }
}

float filter_sample(peak_filter_t* filter, float input_sample)
{
    float tmp_a = 0.0f;
    float tmp_b = 0.0f;
    update_filter_status(filter, input_sample);
    for (uint8_t ii = 0; ii < filter->len_b; ii++)
    {
        tmp_b += filter->b[ii] * filter->x[ii];
    }
    for (uint8_t ii = 0; ii < filter->len_a; ii++)
    {
        tmp_a += filter->a[ii] * filter->y[ii];
    }
    filter->pre_result = tmp_b - tmp_a;
    return filter->pre_result;
}