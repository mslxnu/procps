#ifndef SIGNAL_COMPAT_H
#define SIGNAL_COMPAT_H
#ifdef __APPLE__

#include <signal.h>
#include <errno.h>

// 1. Fix SIGPOLL (macOS uses SIGIO instead for asynchronous I/O events)
#ifndef SIGPOLL
#define SIGPOLL SIGIO
#endif

// 2. Fix _NSIG (macOS standard C library provides NSIG without the underscore)
#ifndef _NSIG
#define _NSIG NSIG
#endif

// 3. Fix Real-Time Signal Macros (macOS doesn't support RT signals, so point them to an empty/fake range)
#ifndef SIGRTMIN
#define SIGRTMIN 32
#endif

#ifndef SIGRTMAX
#define SIGRTMAX 32
#endif

#ifndef __SIGRTMIN
#define __SIGRTMIN SIGRTMIN
#endif

#ifndef __SIGRTMAX
#define __SIGRTMAX SIGRTMAX
#endif

// Dummy union to match POSIX signature if not defined
#ifndef SA_SIGINFO
// union sigval definition fallback handled by system headers
#endif

static __inline
int sigqueue(pid_t pid, int signo, const union sigval value) {
    // macOS does not support queued payloads; fall back to standard kill
    int res = kill(pid, signo);
    return res;
}

#endif /* __APPLE__ */
#endif /* SIGNAL_COMPAT_H */
