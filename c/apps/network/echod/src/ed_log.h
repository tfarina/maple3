/*
 * Odyssey.
 *
 * Scalable PostgreSQL connection pooler.
 */

/*
 * twemcache - Twitter memcached.
 * Copyright (c) 2012, Twitter, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of the Twitter nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ED_LOG_H_
#define ED_LOG_H_

#include <stdarg.h>

typedef enum ed_log_level_e {
  ED_LOG_LEVEL_ERROR  = 0,  /* error conditions */
  ED_LOG_LEVEL_WARN   = 1,  /* warning conditions */
  ED_LOG_LEVEL_NOTICE = 2,  /* normal but significant messages */
  ED_LOG_LEVEL_INFO   = 3,  /* informational messages */
  ED_LOG_LEVEL_DEBUG  = 4,  /* debug messages */
  ED_LOG_LEVEL_MAX
} ed_log_level_t;

typedef enum {
  ED_LOG_PRINT_TIME  = 1 << 0,  /* date/time */
  ED_LOG_PRINT_LEVEL = 1 << 1,  /* log level prefix */
  ED_LOG_PRINT_SRC   = 1 << 2,  /* source location (file, line, function) */
} ed_log_flag_t;

int ed_log_init(char const *filename);
void ed_log_deinit(void);

void ed_log_set_flag(ed_log_flag_t flag);

void ed_log_write(ed_log_level_t level,
	          char const *file,
		  int line,
		  char const *func,
		  char const *format, ...);

#ifdef NDEBUG
#define ed_log_debug(...) do { } while (0)
#else
#define ed_log_debug(...) ed_log_write(ED_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#define ed_log_notice(...) ed_log_write(ED_LOG_LEVEL_NOTICE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ed_log_info(...)   ed_log_write(ED_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ed_log_warn(...)   ed_log_write(ED_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ed_log_error(...)  ed_log_write(ED_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

#endif  /* ED_LOG_H_ */
