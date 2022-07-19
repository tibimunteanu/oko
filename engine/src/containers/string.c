#include "containers/string.h"
#include "core/memory.h"

#include <string.h>

u64 string_length(const char* str) {
    return strlen(str);
}

char* string_duplicate(const char* str) {
    u64 length = string_length(str);
    char* copy = memory_allocate(length + 1, MEMORY_TAG_STRING);
    memory_copy(copy, str, length + 1);
    return copy;
}