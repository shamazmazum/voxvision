/*
  GCD stubs for platforms which does not have it.
*/

#include <unistd.h>
#include <stdlib.h>

#define DISPATCH_QUEUE_PRIORITY_DEFAULT 0L
#define DISPATCH_QUEUE_PRIORITY_HIGH 0L

typedef void *dispatch_queue_t;
typedef void *dispatch_group_t;

static inline void* dispatch_get_global_queue (long priority, unsigned long flags)
{
    return NULL;
}

static inline void dispatch_apply (size_t iterations, dispatch_queue_t queue, void (^block)(size_t))
{
    size_t i;
    for (i=0; i<iterations; i++) block (i);
}

static inline void dispatch_sync (dispatch_queue_t queue, void (^block)(void))
{
    block();
}

static inline void dispatch_async (dispatch_queue_t queue, void (^block)(void))
{
    block();
}

static inline void dispatch_group_async (dispatch_group_t group, dispatch_queue_t queue, void (^block)(void))
{
    block();
}

static inline void dispatch_group_notify (dispatch_group_t group, dispatch_queue_t queue, void (^block)(void))
{
    block();
}
