/*
 * Modified from http://zetcode.com/db/mysqlc/
 */

#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>

static const char kDBHost[] = "localhost";
static const char kDBUser[] = "ken";
static const char kDBPassword[] = "194304";
static const char kDBName[] = "ctestdb";

static void finish_with_error(MYSQL *con) {
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1); 
}

int main(int argc, char **argv) {
  MYSQL *sql = mysql_init(NULL);
  unsigned int port = 0;

  if (sql == NULL) {
    fprintf(stderr, "mysql_init() failed\n");
    exit(1);
  }

  if (mysql_real_connect(sql, kDBHost, kDBUser, kDBPassword, kDBName, port, NULL, 0) == NULL) {
    finish_with_error(sql);
  }

  if (mysql_query(sql, "SELECT * FROM bookmarks")) {
    finish_with_error(sql);
  }

  MYSQL_RES *result = mysql_store_result(sql);

  if (result == NULL) {
    finish_with_error(sql);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result))) {
    for(int i = 0; i < num_fields; i++) {
      printf("%s ", row[i] ? row[i] : "NULL");
    }
    printf("\n");
  }

  mysql_free_result(result);
  mysql_close(sql);

  return 0;
}
