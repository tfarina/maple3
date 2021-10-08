#ifndef STATS_H_
#define STATS_H_

#include <stddef.h>

/**
 * Calculates the minimum value in the specified array.
 *
 * @param data The array.
 * @param size The array size.
 *
 */
double stats_min(double const *data, size_t size);

/**
 * Calculates the maximum value in the specified array.
 *
 * @param data The array.
 * @param size The array size.
 *
 */
double stats_max(double const *data, size_t size);

/**
 * Calculates the sum of all values in the specified array.
 *
 * @param data The array.
 * @param size The array size.
 *
 */
double stats_sum(double const *data, size_t size);

/**
 * Calculates the arithmetic mean of the values in the specified array.
 *
 * @param data The array.
 * @param size The array size.
 *
 */
double stats_avg(double const *data, size_t size);

/**
 * Calculates the variance based on a sample of the values in the
 * specified array.
 *
 * @param data The array.
 * @param size The array size.
 *
 */
double stats_var(double const *data, size_t size);

/**
 * Calculates the standard deviation based on a sample of the values in the
 * in the specified array.
 *
 * @param data The array.
 * @param size The array size.
 *
 */
double stats_stdev(double const *data, size_t size);

#endif /* STATS_H_ */
