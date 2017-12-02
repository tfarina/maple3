#include "os.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatal(const char *msg, ...)
{
        va_list args;

        va_start(args, msg);

        fprintf(stderr, "fatal: ");
        vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");

        va_end(args);

        exit(EXIT_FAILURE);
}

char* xstrdup(char *p)
{
	p = strdup(p);
        if (p == NULL)
                fatal("out of memory");
	return p;
}

void* xrealloc(void *oldptr, size_t size)
{
        void *newptr;

	if ((newptr = realloc(oldptr, size)) == NULL)
                fatal("out of memory");

	return newptr;
}
