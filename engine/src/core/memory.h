#pragma once

#include "defines.h"

typedef enum memory_tag {
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
} memory_tag;

OKO_API b8 memory_initialize(u64* memory_requirement, void* state);
OKO_API void memory_shutdown(void* state);

OKO_API void* memory_allocate(u64 size, memory_tag tag);
OKO_API void memory_free(void* block, u64 size, memory_tag tag);
OKO_API void* memory_zero(void* block, u64 size);
OKO_API void* memory_copy(void* dest, const void* source, u64 size);
OKO_API void* memory_set(void* dest, i32 value, u64 size);

OKO_API char* memory_get_usage_string();

OKO_API u64 memory_get_alloc_count();

// NOTE:
// some low level platform code is still using malloc / free.
// it's ok because they are just part of the startup routine.