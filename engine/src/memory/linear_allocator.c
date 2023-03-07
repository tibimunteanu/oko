#include "linear_allocator.h"

#include "core/memory.h"
#include "core/log.h"

void linear_allocator_create(
    u64 total_size, void* memory, linear_allocator* out_allocator
) {
    if (!out_allocator) {
        OKO_ERROR("out_allocator is not a valid pointer.");
        return;
    }

    if (memory) {
        out_allocator->memory = memory;
    } else {
        out_allocator->memory =
            memory_allocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
    }

    out_allocator->total_size = total_size;
    out_allocator->owns_memory = memory == 0;
    out_allocator->allocated = 0;
}

void linear_allocator_destroy(linear_allocator* allocator) {
    if (!allocator) {
        OKO_ERROR("Linear allocator is not initialized.");
        return;
    }

    if (allocator->owns_memory && allocator->memory) {
        memory_free(
            allocator->memory,
            allocator->total_size,
            MEMORY_TAG_LINEAR_ALLOCATOR
        );
    }

    allocator->memory = 0;
    allocator->total_size = 0;
    allocator->owns_memory = false;
    allocator->allocated = 0;
}

void* linear_allocator_allocate(linear_allocator* allocator, u64 size) {
    if (!allocator || !allocator->memory) {
        OKO_ERROR(
            "linear_allocator_allocate - provided allocator not initialized."
        );
        return 0;
    }

    if (allocator->allocated + size > allocator->total_size) {
        u64 remaining = allocator->total_size - allocator->allocated;
        OKO_ERROR(
            "linear_allocator_allocate - Tried to allocate %lluB, only %lluB remaining.",
            size,
            remaining
        );
        return 0;
    }

    void* block = ((u8*)allocator->memory) + allocator->allocated;
    allocator->allocated += size;
    return block;
}

void linear_allocator_free_all(linear_allocator* allocator) {
    if (!allocator || !allocator->memory) {
        OKO_ERROR(
            "linear_allocator_free_all - provided allocator not initialized."
        );
        return;
    }

    allocator->allocated = 0;
    memory_zero(allocator->memory, allocator->total_size);
}