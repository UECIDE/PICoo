#ifndef _TUNING_H
#define _TUNING_H

// This is how often the core timer interrupts.  The core timer
// switches the thread contexts, and also updates the millisecond
// counters.  Increasing this will allow each thread more time to
// execute at the expense of multi tasking responsivenetss and
// granularity of the timing counters.
// The value is represented in microseconds.

#ifndef CORE_US
# define CORE_US 100
#endif

// This is the overall amount of memory to reserve for thread stacks.
// More = more threads supported, but less space for other data.

#ifndef STACK_SIZE
# define STACK_SIZE 8192
#endif

// How much memory (from the reserved stack memory above) to allocate
// by default to a thread that doesn't specify its own stack size.

#ifndef DEFAULT_STACK_BLOCK
# define DEFAULT_STACK_BLOCK 1024
#endif

#endif
