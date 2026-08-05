#ifndef HUGETLB_COMPAT_H
#define HUGETLB_COMPAT_H

#include <sys/mman.h>
#include <sys/types.h>
#include <mach/vm_statistics.h> // Required for VM_FLAGS_SUPERPAGE definitions

/* Define the Linux flag if missing so code compiles */
#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000 /* Linux value, used here purely as a bitmask token */
#endif

/* 
 * Handle macOS Superpage Flag fallbacks.
 * XNU supports specific sub-allocator flags for superpages depending on architecture.
 */
#ifndef VM_FLAGS_SUPERPAGE_SIZE_2MB
#define VM_FLAGS_SUPERPAGE_SIZE_2MB 0x10000
#endif

/* The Compatibility Interceptor */
static __inline
void* macos_procfs_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    if (flags & MAP_HUGETLB) {
        // Strip the Linux-specific flag to prevent XNU from throwing EINVAL
        flags &= ~MAP_HUGETLB;
        
        // Superpages on macOS require an anonymous, private mapping
        if ((flags & MAP_ANON) && (flags & MAP_PRIVATE)) {
            flags |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
        }
    }
    
    void *result = mmap(addr, len, prot, flags, fd, offset);
    
    /* 
     * Fallback mechanism:
     * If the XNU kernel rejects VM_FLAGS_SUPERPAGE_SIZE_2MB due to memory fragmentation 
     * or lack of entitlement, retry with regular anonymous pages so hugetop doesn't fail.
     */
    if (result == MAP_FAILED && (flags & VM_FLAGS_SUPERPAGE_SIZE_2MB)) {
        flags &= ~VM_FLAGS_SUPERPAGE_SIZE_2MB;
        result = mmap(addr, len, prot, flags, fd, offset);
    }
    
    return result;
}

/* Override the standard mmap call across the tool's codebase */
#define mmap macos_procfs_mmap

#endif /* HUGETLB_COMPAT_H */
