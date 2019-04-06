#ifndef DB_MYSQL_H_
#define DB_MYSQL_H_

#include <mysql/mysql.h>

typedef struct mb_sql_connection_s {
  void *data;
} mb_sql_connection_t;

int db_mysql_alloc(mb_sql_connection_t *conn);
int db_mysql_free(mb_sql_connection_t *conn);

int db_mysql_connect(MYSQL **conn, const char *host, const char *user,
                     const char *password, const char *dbname);

#endif /* DB_MYSQL_H_ */
