#ifndef TPROBES_H
#define TPROBES_H

#ifdef STATISTICS
#include <voxtrees-dtrace.h>
#define WITH_STAT(expr) expr
#else
#define WITH_STAT(expr)
#endif /* STATISTICS */
#endif
