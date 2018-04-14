#ifndef STOCK_H_
#define STOCK_H_

#include <stddef.h>

#include <mysql/mysql.h>

typedef struct {
  char *date;
  double open;
  double high;
  double low;
  double close;
  double adj_close;
  int volume;
} stock_data_t;
 
typedef enum {
  DATE, OPEN, HIGH, LOW, CLOSE, ADJ_CLOSE, VOLUME
} stock_data_field_t;
 
typedef struct {
  char *symbol;
  stock_data_t *ticks;
  size_t ticks_alloc;
  size_t ticks_used;
  stock_data_field_t cur_field;
  int error;
} stock_info_t;

int stock_add_tick(MYSQL *conn, const char *symbol, stock_data_t *tick);
 
#endif  /* STOCK_H_ */
