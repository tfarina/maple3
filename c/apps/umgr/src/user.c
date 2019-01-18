#include "user.h"

#include <stdio.h>

#include "third_party/sqlite/amalgamation/sqlite3.h"

#include "db.h"

/* The name of the user database file.  */
static const char user_db_fname[] = "users.db";

static sqlite3 *user_db;
static sqlite3_stmt *user_insert_stmt;

/* This is the column separator. */
#define SEP_COL "|"

/**
 * Makes sure the 'user' table is created if it does not exist yet.
 * Returns 0 on success, -1 otherwise.
 */
int user_init_database(sqlite3* db) {
  int rc;
  sqlite3_stmt *stmt;
  const char sql[] =
    "CREATE TABLE IF NOT EXISTS user ("
    "  uid INTEGER PRIMARY KEY,"     /* user id */
    "  fname TEXT NOT NULL UNIQUE,"  /* first name */
    "  lname TEXT NOT NULL,"         /* last name */
    "  email TEXT"                   /* email */
    ");";

  if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
    fprintf(stderr, "error creating user table: %s\n", sqlite3_errstr(rc));
    db_close(db);
    return -1;
  }

  rc = sqlite3_step(stmt);

  sql_stmt_free(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "error creating user table: %s\n", sqlite3_errstr(rc));
    db_close(db);
    return -1;
  }

  return 0;
}

user_t *user_alloc(void) {
  user_t *user;

  user = malloc(sizeof(user_t));
  user->fname = NULL;
  user->lname = NULL;
  user->email = NULL;

  return user;
}

int user_exists(const char* email) {
  sqlite3 *user_db;
  sqlite3_stmt *stmt;
  int rc;

  rc = db_open(user_db_fname, &user_db);
  if (rc != SQLITE_OK) {
    return -1;
  }

  if (user_init_database(user_db)) {
    db_close(user_db);
    return -1;
  }

  char* sql = sqlite3_mprintf("SELECT 1 FROM user WHERE email=%Q", email);

  if (sqlite3_prepare_v2(user_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(user_db));
    return -1;
  }

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    rc = 0;
  } else {
    rc = 1;
  }

  sql_stmt_free(stmt);

  db_close(user_db);

  return rc;
}

int user_add(const char* fname,
             const char* lname,
             const char* email) {
  sqlite3 *user_db;
  sqlite3_stmt *stmt;
  int rc;

  rc = db_open(user_db_fname, &user_db);
  if (rc != SQLITE_OK) {
    return -1;
  }

  if (user_init_database(user_db)) {
    db_close(user_db);
    return -1;
  }

  const char *sql = "INSERT INTO user (fname, lname, email) VALUES (?1, ?2, ?3);";

  if (sqlite3_prepare_v2(user_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "error preparing insert statement: %s\n",
            sqlite3_errmsg(user_db));
    db_close(user_db);
    return -1;
  }

  user_insert_stmt = stmt;

  sqlite3_bind_text(stmt, 1, fname, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, lname, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, email, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "error inserting into user table: %s\n",
            sqlite3_errmsg(user_db));
    return -1;
  }

  sql_stmt_free(stmt);

  db_close(user_db);

  return 0;
}

int user_change(const char *fname, const char *email) {
  sqlite3 *user_db;
  sqlite3_stmt *stmt;
  int rc;

  rc = db_open(user_db_fname, &user_db);
  if (rc != SQLITE_OK) {
    return -1;
  }

  if (user_init_database(user_db)) {
    db_close(user_db);
    return -1;
  }

  const char *sql = "UPDATE user SET fname=?1 WHERE email=?2;";

  if (sqlite3_prepare_v2(user_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "error preparing update statement: %s\n",
            sqlite3_errmsg(user_db));
    return -1;
  }

  rc = sqlite3_bind_text(stmt, 1, fname, -1, SQLITE_STATIC);

  if (rc == SQLITE_OK)
    rc = sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "error binding a value for the user table: %s\n",
            sqlite3_errmsg(user_db));
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "error updating user table: %s\n", sqlite3_errmsg(user_db));
    return -1;
  }

  sql_stmt_free(stmt);

  db_close(user_db);

  return 0;
}

int user_delete(const char *email) {
  sqlite3 *user_db;
  sqlite3_stmt *stmt;
  int rc;

  rc = db_open(user_db_fname, &user_db);
  if (rc != SQLITE_OK) {
    return -1;
  }

  if (user_init_database(user_db)) {
    db_close(user_db);
    return -1;
  }

  const char *sql = "DELETE FROM user WHERE email=?1;";

  if (sqlite3_prepare_v2(user_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "error preparing delete statement: %s\n",
            sqlite3_errmsg(user_db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "error deleting user from table: %s\n",
            sqlite3_errmsg(user_db));
    return -1;
  }

  sql_stmt_free(stmt);

  db_close(user_db);

  return 0;
}

alpm_list_t *user_get_records(void) {
  int rc;
  const char *sql;
  sqlite3_stmt *stmt;
  alpm_list_t *users = NULL;

  rc = db_open(user_db_fname, &user_db);
  if (rc != SQLITE_OK) {
    return NULL;
  }

  if (user_init_database(user_db)) {
    db_close(user_db);
    return NULL;
  }

  sql = sqlite3_mprintf("SELECT * FROM user");

  if (sqlite3_prepare_v2(user_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "error preparing select statement: %s\n",
            sqlite3_errmsg(user_db));
    return NULL;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    user_t *user = user_alloc();
    user->fname = strdup((const char *)sqlite3_column_text(stmt, 1));
    user->lname = strdup((const char *)sqlite3_column_text(stmt, 2));
    user->email = strdup((const char *)sqlite3_column_text(stmt, 3));
    users = alpm_list_add(users, user);
  }

  sql_stmt_free(stmt);

  db_close(user_db);

  return users;
}

int user_print_records(void) {
  alpm_list_t *i, *users;

  users = user_get_records();

  for (i = users; i; i = alpm_list_next(i)) {
    user_t *user = i->data;
    printf("%s|%s|%s\n", user->fname, user->lname, user->email);
  }

  return 0;
}
