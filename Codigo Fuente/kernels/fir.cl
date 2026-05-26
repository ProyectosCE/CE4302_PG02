__kernel void fir_filter_bank(
    __global const float* signal,
    __global const float* filters,
    __global float* output,
    const int signal_size,
    const int filter_order,
    const int filter_count
)
{
    int n = get_global_id(0);
    int f = get_global_id(1);

    if (n >= signal_size || f >= filter_count)
    {
        return;
    }

    float acc = 0.0f;

    int filter_base = f * filter_order;
    int output_base = f * signal_size;

    for (int k = 0; k < filter_order; k++)
    {
        if (n >= k)
        {
            acc += filters[filter_base + k] * signal[n - k];
        }
    }

    output[output_base + n] = acc;
}