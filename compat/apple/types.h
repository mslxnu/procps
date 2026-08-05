#ifndef TYPES_COMPAT_H
#define TYPES_COMPAT_H

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* 
 * Core GNU/Linux 64-bit Type Definitions
 * macOS uses 64-bit offsets by default for off_t, so off_t is already 64 bits.
 */
typedef off_t off64_t;
typedef int64_t loff_t;
typedef blkcnt_t blkcnt64_t;

/*
 * Standard 64-bit Function Mapping Redirections
 * Because macOS native calls use 64-bit offsets inherently, we map the 
 * "64" suffixed GNU functions directly to their native equivalents.
 */
#define lseek64       lseek
#define ftruncate64   ftruncate
#define truncate64    truncate
#define mmap64        mmap

/* File-system and I/O function mapping */
#define stat64        stat
#define fstat64       fstat
#define lstat64       lstat
#define fopen64       fopen
#define freopen64     freopen
#define fseeko64      fseeko
#define ftello64      ftello

/* Struct type mappings */
#define stat64_t      stat

/*
 * Feature Macro Overrides
 * Some Linux packages explicitly look for these macro variables to 
 * confirm large file support (LFS) configurations.
 */
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif

#endif /* TYPES_COMPAT_H */
