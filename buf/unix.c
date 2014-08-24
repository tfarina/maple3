#include "unix.h"

#include <stdlib.h>
#include <string.h>

void fatal(char *msg, ...)
{
}

void xfree(void* p)
{
        free(p);
}

char* xstrdup(char *p)
{
	p = strdup(p);
        if (p == nil)
                fatal("out of memory");
	return p;
}

void* xrealloc(void *p, int n)
{
	p = realloc(p, n);
        if (p == nil)
                fatal("out of memory");
	return p;
}

int xstrlen(char *p)
{
	return strlen(p);
}

char* xstrstr(char *a, char *b)
{
	return strstr(a, b);
}

void xmemmove(void *dst, void *src, int n)
{
	memmove(dst, src, n);
}

int xmemcmp(void *a, void *b, int n)
{
	return memcmp(a, b, n);
}
