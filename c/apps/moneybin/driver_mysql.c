#include "driver_mysql.h"

#include <stdio.h>

#include "sql.h"

typedef struct db_mysql_data_s {
  MYSQL *mysql;
  MYSQL_RES *result;
} db_mysql_data_t;

static int db_mysql_alloc(mb_sql_connection_t *conn)
{
  db_mysql_data_t *data;

  data = calloc(1, sizeof(*data));
  if (data == NULL) {
    return -1 /*ENOMEM*/;
  }

  conn->data = data;

  return 0;
}

static int db_mysql_free(mb_sql_connection_t *conn)
{
  db_mysql_data_t *data;

  data = conn->data;

  free(data->mysql);
  free(data);

  conn->data = NULL;

  return 0;
}

int db_mysql_connect(MYSQL **conn, const char *host, int unsigned port,
                     const char *user, const char *password, const char *dbname)
{
  char *unix_socket_name = NULL;
  unsigned long client_flags = 0;

  if ((*conn = mysql_init(NULL)) == NULL) {
    fprintf(stderr, "mysql: unable to allocate memory for database connection.\n");
    return -1;
  }

  if (mysql_real_connect(*conn, host, user, password, dbname, port,
                         unix_socket_name, client_flags) == NULL) {
    fprintf(stderr, "mysql: connection to database failed: %s\n", mysql_error(*conn));
    mysql_close(*conn);
    return -1;
  }

  return 0;
}