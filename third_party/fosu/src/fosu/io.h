#ifndef FOSU_IO_H
#define FOSU_IO_H

// File input through the C library (fopen/fread): portable, no OS headers.

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <fosu/span.h>

namespace fosu {

    // The parser requires kBufferPadding readable zero bytes past the end of the input so
    // speculative reads never run off the buffer. 128 bounds the worst-case speculative read of the
    // timing point parser on garbage input (~90 bytes past a line start near EOF).
    const size_t kBufferPadding = 128;

    inline bool can_pad_input(size_t size) {
        return size <= std::numeric_limits<size_t>::max() - kBufferPadding;
    }

    namespace internal {

        // Opens `path` for reading and reports its size. Returns null on failure (errno set).
        inline std::FILE* open_input_file(const char* path, size_t& size) {
            std::FILE* file = std::fopen(path, "rb");
            if (!file) {
                return 0;
            }
            if (std::fseek(file, 0, SEEK_END) != 0) {
                const int error = errno;
                std::fclose(file);
                errno = error ? error : EIO;
                return 0;
            }
            const long length = std::ftell(file);
            if (length < 0 || std::fseek(file, 0, SEEK_SET) != 0) {
                const int error = errno;
                std::fclose(file);
                errno = error ? error : EIO;
                return 0;
            }
            size = static_cast<size_t>(length);
            return file;
        }

        // Reads exactly `size` bytes into `data`. Returns false on a short read or an I/O error.
        inline bool read_input_file(std::FILE* file, char* data, size_t size) {
            size_t got = 0;
            while (got < size) {
                const size_t count = std::fread(data + got, 1, size - got, file);
                if (count == 0) {
                    if (!errno) {
                        errno = EIO;
                    }
                    return false;
                }
                got += count;
            }
            return true;
        }

    } // namespace internal

    // A padded copy of a file or string (malloc'd), for tests, tools and read_file_padded.
    struct FileBuffer {
        char* data;
        size_t size;
        size_t capacity; // allocated bytes, padding included

        FileBuffer()
            : data(0),
              size(0),
              capacity(0) {}
        ~FileBuffer() { std::free(data); }

        bool valid() const { return data != 0; }
        StringView view() const { return StringView(data, size); }

        bool reserve(size_t length) {
            if (!can_pad_input(length)) {
                return false;
            }
            if (capacity >= length + kBufferPadding) {
                return true;
            }
            char* grown = static_cast<char*>(std::realloc(data, length + kBufferPadding));
            if (!grown) {
                return false;
            }
            data = grown;
            capacity = length + kBufferPadding;
            return true;
        }

    private:
        FileBuffer(const FileBuffer&);
        FileBuffer& operator=(const FileBuffer&);
    };

    // Reads `path` into `buf`, reusing its allocation when large enough (grow-only).
    inline bool read_into(const char* path, FileBuffer& buf) {
        buf.size = 0;
        size_t length;
        std::FILE* file = internal::open_input_file(path, length);
        if (!file) {
            return false;
        }
        if (!buf.reserve(length)) {
            std::fclose(file);
            errno = ENOMEM;
            return false;
        }
        if (!internal::read_input_file(file, buf.data, length)) {
            const int error = errno;
            std::fclose(file);
            errno = error;
            return false;
        }
        std::fclose(file);
        std::memset(buf.data + length, 0, kBufferPadding);
        buf.size = length;
        return true;
    }

    // Copies an in-memory string into a padded buffer.
    inline bool make_padded(StringView content, FileBuffer& buf) {
        buf.size = 0;
        if (!buf.reserve(content.size())) {
            return false;
        }
        if (!content.empty()) {
            std::memcpy(buf.data, content.data(), content.size());
        }
        std::memset(buf.data + content.size(), 0, kBufferPadding);
        buf.size = content.size();
        return true;
    }

} // namespace fosu

#endif
