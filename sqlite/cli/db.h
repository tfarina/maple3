#ifndef CLI_DB_H_
#define CLI_DB_H_

typedef struct sqlite3 sqlite3;

/* Returns a pointer to sqlite3. */
sqlite3* db_open(const char* db_file);

/* Makes sure the 'user' table is created if it does not exist yet.
 * Returns 0 on success, -1 otherwise. */
int db_user_create_table(sqlite3* db);

#endif  // CLI_DB_H_
