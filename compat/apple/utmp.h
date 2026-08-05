#ifndef UTMP_COMPAT_H
#define UTMP_COMPAT_H

#ifdef __APPLE__
#include <utmp.h>
#include <utmpx.h>

// 1. Map old legacy utmp naming to standard modern utmpx structure
#define utmp     utmpx
#define ut_name  ut_user

// 2. Map legacy utmp functions directly to macOS utmpx equivalents
#define setutent  setutxent
#define getutent  getutxent
#define endutent  endutxent
#endif

#endif /* UTMP_COMPAT_H */
