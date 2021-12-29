#ifndef _OS_PATH_H_INCLUDED_
#define _OS_PATH_H_INCLUDED_ 1

/**
 * Returns the last element of path.
 *
 * @param[in] path A path name.
 */
char const *os_path_basename(char const *path);

/**
 * Joins directory with file name into a single path, separating them with an
 * Linux specific separator.
 *
 * @param[in] dir A directory path.
 * @param[in] file A file name.
 */
char *os_path_join(char *dir, char *file);

#endif  /* !defined(_OS_PATH_H_INCLUDED_) */
