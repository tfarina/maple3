#include "ta.h"

/**
 * Calculates a exponential moving average.
 *
 * @param[in]  input
 * @param[in]  size
 * @param[in]  period
 * @param[out] output
 * @return 0 on success, negative number (< 0) on error.
 */
int ind_ema(double const *input, size_t size, int period, double *output)
{
  float val;
  int i;
  float k;

  k = 2 / ((float)period + 1); /* 2 is the smoothing factor */

  val = *input;
  *output++ = val;

  for (i = 1; i < size; i++) {
    /* EMA = (close - prev_day_ema) * k + prev_day_ema */
    val = (*(input + i) - val) * k + val;

    *output++ = val;
  }

  return 0;
}
