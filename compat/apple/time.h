#ifndef TIME_COMPAT_H
#define TIME_COMPAT_H

#ifdef __APPLE__
#include <time.h>
#include <mach/mach_time.h>

/* 1. Define missing Linux clock identifiers for compilation */
#ifndef CLOCK_BOOTTIME
#define CLOCK_BOOTTIME CLOCK_MONOTONIC
#endif

/* 2. Optional: High-accuracy wrapper if you actually need sleep-time tracking */
static __inline
int macos_get_boottime(struct timespec *ts) {
    static mach_timebase_info_data_t timebase_info;
    
    // Initialize timebase factor once
    if (timebase_info.denom == 0) {
        mach_timebase_info(&timebase_info);
    }
    
    // mach_absolute_time() continues to tick during sleep on modern macOS
    uint64_t mach_time = mach_absolute_time();
    uint64_t nanos = mach_time * timebase_info.numer / timebase_info.denom;
    
    ts->tv_sec = nanos / 1000000000ULL;
    ts->tv_nsec = nanos % 1000000000ULL;
    return 0;
}

/* 3. Redirect clock_gettime calls safely */
#define clock_gettime(clk_id, ts) \
    ((clk_id) == CLOCK_BOOTTIME ? macos_get_boottime(ts) : clock_gettime(clk_id, ts))

#endif /* __APPLE__ */

#endif /* TIME_COMPAT_H */
