#include "fstrutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
str_is_null_or_empty(char const *str)
{
  if (!str || *str == '\0') {
    return 1;
  }

  return 0;
}

/**
 * Tests if this string starts with the specified prefix.
 *
 * @param[in] str The string.
 * @param[in] prefix The prefix to compare.
 * @return true if prefix matches the beginning of this string; false otherwise.
 */
int
str_startswith(char const *str, char const *prefix)
{
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

char *
str_substring(char const *str, int start, int end)
{
  int len;
  char *result;

  len = end - start;
  result = malloc(sizeof(char) * len + 1);
  if (result == NULL) {
    return NULL;
  }
  memcpy(result, str + start, len);
  result[len] = '\0';

  return result;
}

#undef IS_WHITESPACE
#define IS_WHITESPACE(x) ((x) == ' ' || (x) == '\t')

/**
 * Returns a string whose value is this string, with all leading white
 * space removed.
 *
 * NOTE: The string is changed in place.
 */
char *
str_strip_leading(char *str)
{
  char *cur;

  /*
   * 1. Initialize cur to str. This makes cur point to the first character of
   * str.
   * 2. Check if the current character is whitespace.
   * 3. While it is, keeping incrementing the pointer to the next character.
   */
  for (cur = str;
       *cur && IS_WHITESPACE(*cur);
       cur++)
    ;

  memmove(str, cur, strlen(cur) + 1);

  return str;
}

/**
 * Returns a string whose value is this string, with all trailing white space
 * removed.
 *
 * NOTE: The string is changed in place.
 */
char *
str_strip_trailing(char *str)
{
  char *cur;

  /*
   * 1. Make cur point to the last character of str.
   * 2. Check if the current character is whitespace.
   * 3. If current character is whitespace, zero it.
   * 4. Decrement the current pointer to point the next character.
   * After zeroing all the trailing whitespace characters, return the
   * result string.
   */
  for (cur = str + strlen(str) - 1;
       cur >= str && IS_WHITESPACE(*cur);
       cur--) {
    *cur = '\0';
  }

  return str;
}

/**
 * Returns a string whose value is this string, with all leading and trailing
 * white space removed.
 *
 * NOTE: The string is changed in place.
 */
char *
str_strip(char *str)
{
  str_strip_leading(str);
  str_strip_trailing(str);

  return str;
}
