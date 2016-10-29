#ifndef UNIX_H_
#define UNIX_H_

// fatal prints an error message to standard error and exits.
void fatal(char *msg, ...);

// xfree frees the result returned by xmalloc, xstrdup, or xrealloc.
void xfree(void *p);

// xstrdup returns a newly allocated copy of p.
// It calls fatal if it runs out of memory.
char *xstrdup(char *p);

// xrealloc grows the allocation p to n bytes and
// returns the new (possibly moved) pointer.
// It calls fatal if it runs out of memory.
void *xrealloc(void *p, int n);

#endif  // UNIX_H_
