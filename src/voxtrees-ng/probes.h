#ifndef NG_PROBES_H
#define NG_PROBES_H

#ifdef STATISTICS
#include <voxtrees-ng-dtrace.h>
#define WITH_STAT(expr) expr
#else
#define WITH_STAT(expr)
#endif /* STATISTICS */
#endif
