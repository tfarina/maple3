#include <stdio.h>

#include "csv_helper.h"
#include "stats.h"

int main(int argc, char **argv) {
  double *close;
  int num_rows;
  double min_close;
  double max_close;
  double avg;

  if (argc != 2) {
    fputs("usage: csvstats filename.csv\n", stderr);
    return 1;
  }

  load_close_prices(argv[1]);

  close = csv_close_prices();
  num_rows = csv_num_rows();

  stats_min(close, num_rows, &min_close);
  stats_max(close, num_rows, &max_close);
  stats_average(close, num_rows, &avg);

  printf("Min: %f\n", min_close);
  printf("Max: %f\n", max_close);
  printf("Avg: %f\n", avg);

  return 0;
}
