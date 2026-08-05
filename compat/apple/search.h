#ifndef SEARCH_COMPAT_H
#define SEARCH_COMPAT_H
#if defined(__APPLE__)

#include <search.h>
#include <stdlib.h>
#include <string.h>

/* Drop-in replacement for the opaque GNU hsearch_data structure */
struct hsearch_data {
    ENTRY *entries;
    size_t size;
    size_t filled;
};

/**
 * Initializes the re-entrant hash table.
 * Returns non-zero on success, 0 on failure.
 */
static inline
int hcreate_r(size_t nel, struct hsearch_data *htab) {
    if (!htab) return 0;
    
    // Use a simple power-of-two allocation or rough prime estimation
    size_t alloc_size = 16;
    while (alloc_size < (nel * 4 / 3)) { // Maintain a < 75% load factor
        alloc_size <<= 1;
    }
    
    htab->entries = (ENTRY *)calloc(alloc_size, sizeof(ENTRY));
    if (!htab->entries) return 0;
    
    htab->size = alloc_size;
    htab->filled = 0;
    return 1;
}

/**
 * Frees the internal table structure.
 * Note: Like GLIBC, this does not free user-allocated key/data pointers.
 */
static inline
void hdestroy_r(struct hsearch_data *htab) {
    if (htab && htab->entries) {
        free(htab->entries);
        htab->entries = NULL;
        htab->size = 0;
        htab->filled = 0;
    }
}

/**
 * Re-entrant hash table search and insertion using FNV-1a hash and linear probing.
 * Returns non-zero on success, 0 on failure.
 */
static inline
int hsearch_r(ENTRY item, ACTION action, ENTRY **retval, struct hsearch_data *htab) {
    if (!htab || !htab->entries || !item.key || !retval) return 0;

    // FNV-1a String Hashing
    unsigned long long hash = 14695981039346656037ULL;
    for (const char *p = item.key; *p; p++) {
        hash ^= (unsigned char)*p;
        hash *= 1099511628211ULL;
    }

    size_t mask = htab->size - 1;
    size_t idx = (size_t)(hash & mask);
    size_t start_idx = idx;

    while (1) {
        ENTRY *slot = &htab->entries[idx];

        // Slot is empty
        if (slot->key == NULL) {
            if (action == FIND) {
                *retval = NULL;
                return 0; // Not found
            }
            
            // Insert action
            if (htab->filled >= htab->size * 3 / 4) {
                *retval = NULL; // Table full (prevents infinite loops)
                return 0; 
            }
            
            slot->key = item.key;
            slot->data = item.data;
            htab->filled++;
            *retval = slot;
            return 1;
        }

        // Slot matches the target key
        if (strcmp(slot->key, item.key) == 0) {
            *retval = slot;
            return 1;
        }

        // Collision: Linear probe to next index
        idx = (idx + 1) & mask;
        if (idx == start_idx) {
            break; // Entire table searched
        }
    }

    *retval = NULL;
    return 0;
}

#endif /* __APPLE__ */
#endif /* SEARCH_COMPAT_H */