#include "filesystem.h"

#include "core/log.h"
#include "core/memory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

b8 filesystem_exists(const char* path) {
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

b8 filesystem_open(
    const char* path, file_modes mode, b8 binary, file_handle* out_handle
) {
    out_handle->is_valid = false;
    out_handle->handle = 0;

    const char* mode_str;

    if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
        mode_str = binary ? "w+b" : "w+";
    } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
        mode_str = binary ? "rb" : "r";
    } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
        mode_str = binary ? "wb" : "w";
    } else {
        OKO_ERROR(
            "Invalid file mode passed while trying to open file: '%s'", path
        );
        return false;
    }

    // attempt to open the file
    FILE* file = fopen(path, mode_str);
    if (!file) {
        OKO_ERROR("Unable to open file: '%s'", path);
        return false;
    }

    out_handle->handle = file;
    out_handle->is_valid = true;

    return true;
}

void filesystem_close(file_handle* handle) {
    if (handle->is_valid) {
        fclose((FILE*)handle->handle);
        handle->handle = 0;
        handle->is_valid = false;
    }
}

b8 filesystem_read_line(file_handle* handle, char** line_buf) {
    if (!handle->is_valid) {
        OKO_ERROR("Invalid file handle passed to filesystem_read_line.");
        return false;
    }

    char buffer[32000];
    if (fgets(buffer, 3200, (FILE*)handle->handle) == 0) {
        OKO_ERROR("Unable to read line from file into the buffer.");
        return false;
    }

    u64 length = strlen(buffer);
    *line_buf = memory_allocate((sizeof(char) * length) + 1, MEMORY_TAG_STRING);
    strcpy(*line_buf, buffer);
    return true;
}

b8 filesystem_write_line(file_handle* handle, const char* line) {
    if (!handle->handle) {
        OKO_ERROR("Invalid file handle passed to filesystem_write_line.");
        return false;
    }

    i32 result = fputs(line, (FILE*)handle->handle);
    if (result != EOF) {
        result = fputc('\n', (FILE*)handle->handle);
    }

    // flush the file to ensure the line is written
    fflush((FILE*)handle->handle);
    return result != EOF;
}

b8 filesystem_read(
    file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read
) {
    if (!handle->is_valid) {
        OKO_ERROR("Invalid file handle passed to filesystem_read.");
        return false;
    }

    u64 bytes_read = fread(out_data, 1, data_size, (FILE*)handle->handle);
    if (out_bytes_read) {
        *out_bytes_read = bytes_read;
    }

    return bytes_read == data_size;
}

b8 filesystem_read_all_bytes(
    file_handle* handle, u8** out_bytes, u64* out_bytes_read
) {
    if (!handle->is_valid) {
        OKO_ERROR("Invalid file handle passed to filesystem_read_all_bytes.");
        return false;
    }

    // get the file size
    fseek((FILE*)handle->handle, 0, SEEK_END);
    u64 file_size = ftell((FILE*)handle->handle);
    rewind((FILE*)handle->handle);

    *out_bytes = memory_allocate(file_size, MEMORY_TAG_STRING);
    *out_bytes_read = fread(*out_bytes, 1, file_size, (FILE*)handle->handle);

    return *out_bytes_read == file_size;
}

b8 filesystem_write(
    file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written
) {
    if (!handle->is_valid) {
        OKO_ERROR("Invalid file handle passed to filesystem_write.");
        return false;
    }

    u64 bytes_written = fwrite(data, 1, data_size, (FILE*)handle->handle);
    if (out_bytes_written) {
        *out_bytes_written = bytes_written;
    }

    return bytes_written == data_size;
}