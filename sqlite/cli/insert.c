#include <stdio.h>

#include <sqlite3.h>

#include "db.h"

// http://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm

static int db_user_init_table(sqlite3* db) {
  const char* sql =
    "CREATE TABLE IF NOT EXISTS 'user' ("
    "  uid INTEGER PRIMARY KEY," /* User ID */
    "  login TEXT UNIQUE,"       /* login name of the user */
    "  pw TEXT,"                 /* password */
    "  email TEXT"               /* e-mail */
    ");";

  if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
    fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  return 0;
}

/* Returns 0 if the user does not exists, otherwise returns 1. */
static int db_user_exists(sqlite3* db, const char* username) {
  sqlite3_stmt* stmt;
  int rc;

  char* sql = sqlite3_mprintf("SELECT 1 FROM user WHERE login=%Q", username);

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    rc = 0;
  } else {
    rc = 1;
  }

  sqlite3_finalize(stmt);
  return rc;
}

static int db_user_add(sqlite3* db,
                       const char* username,
                       const char* password,
                       const char* email) {
  sqlite3_stmt* stmt;

  const char *sql = "INSERT INTO user (login, pw, email) VALUES (?1, ?2, ?3);";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, email, -1, SQLITE_STATIC);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return 0;
}

int main(int argc, char* argv[]) {
  sqlite3* db;

  if (argc != 4) {
    printf("usage: %s USERNAME PASSWORD E-MAIL\n", argv[0]);
    return -1;
  }

  db = db_open("users.db");

  if (db_user_init_table(db)) {
    sqlite3_close(db);
    return -1;
  }

  if (db_user_exists(db, argv[1])) {
    fprintf(stderr, "%s: user (%s) already exists\n", argv[0], argv[1]);
    sqlite3_close(db);
    return -1;
  }

  if (db_user_add(db, argv[1], argv[2], argv[3])) {
    sqlite3_close(db);
    return -1;
  }

  sqlite3_close(db);

  return 0;
}
