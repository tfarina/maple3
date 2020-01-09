#ifndef MSG_H_
#define MSG_H_

#include "ec_macros.h"

void fatal(const char *fmt, ...) EC_FORMAT_PRINTF(1, 2) EC_NORETURN;
void error(const char *fmt, ...) EC_FORMAT_PRINTF(1, 2);
void warn(const char *fmt, ...) EC_FORMAT_PRINTF(1, 2);
void info(const char *fmt, ...) EC_FORMAT_PRINTF(1, 2);

#endif  /* MSG_H_ */