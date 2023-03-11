#pragma once

#include "defines.h"

// for non-pointer types, the table retains a copy of the value.
// for pointer types, make sure to use the _ptr setter and getter.
// table does not take ownership of pointers or associated memory.
typedef struct hashtable {
    u64 element_size;
    u32 element_count;
    b8 is_pointer_type;
    void* memory;
} hashtable;

OKO_API void hashtable_create(
    u64 element_size,
    u32 element_count,
    void* memory,
    b8 is_pointer_type,
    hashtable* out_hashtable
);

OKO_API void hashtable_destroy(hashtable* hashtable);

OKO_API b8 hashtable_set(hashtable* hashtable, const char* name, void* value);

OKO_API b8
hashtable_set_ptr(hashtable* hashtable, const char* name, void** value);

OKO_API b8
hashtable_get(hashtable* hashtable, const char* name, void* out_value);

OKO_API b8
hashtable_get_ptr(hashtable* hashtable, const char* name, void** out_value);

OKO_API b8 hashtable_fill(hashtable* hashtable, void* value);