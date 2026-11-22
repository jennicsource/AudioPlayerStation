#include <stdint.h>
#include <stddef.h>
#include <math.h>

typedef struct {
    double threshold;       /* Sample units */
    double inverse_ratio;
    double attack_coeff;
    double release_coeff;
    double gain;            /* Persistent state */
} Compressor;

static double time_coefficient(double sample_rate, double time_ms)
{
    return time_ms == 0.0
        ? 0.0
        : exp(-1000.0 / sample_rate / time_ms);
}


/* Valid parameters:
 * sample_rate > 0
 * 0 < threshold <= 1 (fraction of full scale)
 * ratio >= 1
 * attack_ms, release_ms >= 0
 *
 * Returns 1 on success, 0 for invalid parameters.
 */
int compressor_init(Compressor *c, double sample_rate,
                    double threshold, double ratio,
                    double attack_ms, double release_ms)
{
    if (!c || !isfinite(sample_rate) || sample_rate <= 0.0 ||
        !isfinite(threshold) || threshold <= 0.0 || threshold > 1.0 ||
        !isfinite(ratio) || ratio < 1.0 ||
        !isfinite(attack_ms) || attack_ms < 0.0 ||
        !isfinite(release_ms) || release_ms < 0.0)
        return 0;

    c->threshold = threshold * 2147483648.0;
    c->inverse_ratio = 1.0 / ratio;
    c->attack_coeff = time_coefficient(sample_rate, attack_ms);
    c->release_coeff = time_coefficient(sample_rate, release_ms);
    c->gain = 1.0;
    return 1;
}


static int32_t to_int32(double value)
{
    if (value >= (double)INT32_MAX) return INT32_MAX;
    if (value <= (double)INT32_MIN) return INT32_MIN;
    return (int32_t)llround(value);
}


/* frames = number of stereo pairs.
 * State must already be initialized.
 */
uint8_t compressor_process(Compressor *c, int32_t *buffer, size_t frames)
{
    uint8_t compressed = 0;
    
    for (size_t i = 0; i < frames; ++i ) 
    {
        /* Convert before fabs: integer abs(INT32_MIN) overflows. */
        double left  = (double)buffer[i * 2];
        double right = (double)buffer[i * 2 + 1];

        double peak  = fmax(fabs(left), fabs(right));
        double target_gain = 1.0;

        if (peak > c->threshold) {
            double target_peak = c->threshold + (peak - c->threshold) * c->inverse_ratio;

            target_gain = target_peak / peak;

            compressed = 1;
        }

        /* Decreasing gain uses attack; increasing gain uses release. */
        double a = target_gain < c->gain
                 ? c->attack_coeff
                 : c->release_coeff;

        c->gain = a * c->gain + (1.0 - a) * target_gain;

        /* Apply the same gain to both channels. */
        buffer[i * 2]     = to_int32(left  * c->gain);
        buffer[i * 2 + 1] = to_int32(right * c->gain);
    }

    return compressed;
}