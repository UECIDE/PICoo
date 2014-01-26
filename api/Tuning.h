#ifndef _TUNING_H
#define _TUNING_H

// This is how often the core timer interrupts.  The core timer
// switches the thread contexts, and also updates the millisecond
// counters.  Increasing this will allow each thread more time to
// execute at the expense of multi tasking responsivenetss and
// granularity of the millisecond counters.

#ifndef CORE_TICK_RATE
# define CORE_TICK_RATE 40000
#endif

#ifndef STACK_SIZE
# define STACK_SIZE 8192
#endif

#endif
