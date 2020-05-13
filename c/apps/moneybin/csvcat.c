#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fstrutils.h"
#include "vector.h"
#include "third_party/libcsv/csv.h"

typedef enum return_code_e {
  RC_SUCCESS,    /* No error */
  RC_ERROR,
} return_code_t;

typedef struct csv_state_s {
  long unsigned fields;
  long unsigned rows;
  long unsigned columns;

  int ignore_first_line;

  /**
   * Which line (row) we are in the parsing process.
   */
  int row;

  /**
   * The next field (column number) to parse.
   */
  int column;

  double *open;
  double *high;
  double *low;
  double *close;
  double *adjclose;
  int *volume;
} csv_state_t;

typedef enum csv_column_e {
  CSV_COLUMN_DATE,
  CSV_COLUMN_OPEN,
  CSV_COLUMN_HIGH,
  CSV_COLUMN_LOW,
  CSV_COLUMN_CLOSE,
  CSV_COLUMN_ADJ_CLOSE,
  CSV_COLUMN_VOLUME
} csv_column_t;

static void print_matrix(csv_state_t *m) {
  int i;

  for (i = 0; i < m->rows - 1; i++) {
    printf("%f ", m->open[i]);
    printf("%f ", m->high[i]);
    printf("%f ", m->low[i]);
    printf("%f ", m->close[i]);
    printf("%f ", m->adjclose[i]);
    printf("%d ", m->volume[i]);
    putc('\n', stdout);
  }
}

static char *parse_str(char const *field, size_t length, return_code_t *rc) {
  if (length > 0) {
    char *str = (char *)malloc((length + 1) * sizeof(char));
    strncpy(str, field, length + 1);

    *rc = RC_SUCCESS;
    return str;
  }

  *rc = RC_ERROR;
  return NULL;
}

static double parse_price(char const *field, size_t length, return_code_t *rc) {
  char *endptr;
  double price;

  price = (double)strtod(field, &endptr);
  if (length > 0 && (endptr == NULL || *endptr == '\0')) {
    *rc = RC_SUCCESS;
    return price;
  }

  *rc = RC_ERROR;
  return -1.0f;
}

static void csv_field_count_cb(void *field, size_t field_length, void *data) {
  csv_state_t *state = (csv_state_t *)data;

  if (state->ignore_first_line && state->rows == 0) {
    return;
  }

  state->fields++;
}

static void csv_row_count_cb(int c, void *data) {
  csv_state_t *state = (csv_state_t *)data;

  state->rows++;
}

/**
 * This functions is called each time a new field has been found.
 */
static void csv_read_field_cb(void *field, size_t field_length, void *data) {
  csv_state_t *state = (csv_state_t *)data;
  char *buffer;
  return_code_t rc;

  if (state->ignore_first_line) {
    return;
  }

  buffer = (char *)malloc((field_length + 1) * sizeof(char));
  strncpy(buffer, field, field_length);
  buffer[field_length] = '\0';

  switch (state->column) {
  case CSV_COLUMN_DATE:
    break;
  case CSV_COLUMN_OPEN:
    state->open[state->row] = parse_price(buffer, field_length, &rc);
    break;
  case CSV_COLUMN_HIGH:
    state->high[state->row] = parse_price(buffer, field_length, &rc);
    break;
  case CSV_COLUMN_LOW:
    state->low[state->row] = parse_price(buffer, field_length, &rc);
    break;
  case CSV_COLUMN_CLOSE:
    state->close[state->row] = parse_price(buffer, field_length, &rc);
    break;
  case CSV_COLUMN_ADJ_CLOSE:
    state->adjclose[state->row] = parse_price(buffer, field_length, &rc);
    break;
  case CSV_COLUMN_VOLUME:
    state->volume[state->row] = atoi(buffer);
    break;
  default:
    rc = RC_ERROR;
    break;
  }

  free(buffer);

  state->column++;
}

/**
 * This functions is called each time a new row has been found.
 */
static void csv_read_row_cb(int c, void *data) {
  csv_state_t *state = (csv_state_t *)data;

  if (state->ignore_first_line) {
    state->ignore_first_line = 0;
    return;
  }

  state->row++;
  state->column = 0;
}

static int csv2matrix(char const *filename, csv_state_t *state) {
  FILE* fp;
  struct csv_parser parser;
  char buf[1024];
  size_t bytes_read;
  long unsigned numrows;
  int i;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file %s: %s\n", filename, strerror(errno));
    return -1;
  }

  state->fields = 0;
  state->rows = 0;
  state->columns = 0;
  state->ignore_first_line = 1;
  state->row = 0;
  state->column = 0;

  if (csv_init(&parser, CSV_STRICT | CSV_APPEND_NULL | CSV_STRICT_FINI) != 0) {
    fprintf(stderr, "Failed to initialize csv parser\n");
    return -1;
  }

  /* First pass to count the total number of rows and fields. */
  while ((bytes_read = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
    if (csv_parse(&parser, buf, bytes_read, csv_field_count_cb, csv_row_count_cb, state) != bytes_read) {
      fprintf(stderr, "Error while parsing %s: %s\n", filename, csv_strerror(csv_error(&parser)));
      csv_free(&parser);
      fclose(fp);
      return -1;
    }
  }
  csv_fini(&parser, csv_field_count_cb, csv_row_count_cb, state);

  numrows = state->rows - (state->ignore_first_line ? 1 : 0);
  state->columns = state->fields / numrows;

  state->open = malloc(sizeof(double) * numrows);
  state->high = malloc(sizeof(double) * numrows);
  state->low = malloc(sizeof(double) * numrows);
  state->close = malloc(sizeof(double) * numrows);
  state->adjclose = malloc(sizeof(double) * numrows);
  state->volume = malloc(sizeof(int) * numrows);

  if (fp) {
    fseek(fp, 0, SEEK_SET);
  }

  while ((bytes_read = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
    if (csv_parse(&parser, buf, bytes_read, csv_read_field_cb, csv_read_row_cb, state) != bytes_read) {
      fprintf(stderr, "Error while parsing %s: %s\n", filename, csv_strerror(csv_error(&parser)));
      csv_free(&parser);
      fclose(fp);
      return -1;
    }
  }

  csv_fini(&parser, csv_read_field_cb, csv_read_row_cb, state);
  csv_free(&parser);

  if (ferror(fp)) {
    fprintf(stderr, "error reading file %s\n", filename);
    fclose(fp);
    return -1;
  }

  fclose(fp);

  return 0;
}

int main(int argc, char **argv) {
  int err;
  char *filename;
  csv_state_t m;

  if (argc != 2) {
    fputs("usage: csvcat filename.csv\n", stderr);
    return 1;
  }

  filename = f_strdup(argv[1]);

  err = csv2matrix(filename, &m);
  if (err < 0) {
    return err;
  }

  print_matrix(&m);

  free(filename);

  return 0;
}
