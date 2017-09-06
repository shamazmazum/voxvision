#ifndef RPROBES_H
#define RPROBES_H

#ifdef STATISTICS
#include <voxrnd-dtrace.h>
#define WITH_STAT(expr) expr
#else
#define WITH_STAT(expr)
#endif /* STATISTICS */
#endif
