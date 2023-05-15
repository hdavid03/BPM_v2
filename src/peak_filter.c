#include <peak_filter.h>
#include <stdint.h>

static double A_COEFFS[] = {-1.998072905031434, 0.998112346058487};
static double B_COEFFS[] = {9.438269707564562e-04, 0.0, -9.438269707564562e-04};
static double X[] = {0.0, 0.0, 0.0};
static double Y[] = {0.0, 0.0};
static uint8_t TAP_NUM  = 3;

static void update_filter_status(peak_filter_t*, double);

void peak_filter_init(peak_filter_t* filter)
{
    filter->a = A_COEFFS;
    filter->len_a = TAP_NUM - 1;
    filter->b = B_COEFFS;
    filter->len_b = TAP_NUM;
    filter->x = X;
    filter->y = Y;
    filter->pre_result = 0.0;
}

static void update_filter_status(peak_filter_t* filter, double input_sample)
{
    double tmp = 0.0;
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

double filter_sample(peak_filter_t* filter, double input_sample)
{
    double tmp_a = 0.0;
    double tmp_b = 0.0;
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