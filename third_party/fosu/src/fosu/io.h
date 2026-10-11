#ifndef FOSU_IO_H
#define FOSU_IO_H

// File input through the OS (POSIX open/read/fstat, Windows CreateFileW), matching upstream's
// internal::InputFile API. The FileBuffer helpers below are port-only: the parser, the tests and
// the tools use them for padded in-memory input.

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>

#if defined(_WIN32)
#include <algorithm>
#include <new>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <fosu/nonstd_config.h>
#include <fosu/types.h>

namespace fosu {
    // The parser requires kBufferPadding readable zero bytes past the end of the input so
    // speculative reads never run off the buffer. 128 bounds the worst-case speculative read of the
    // timing point parser on garbage input (~90 bytes past a line start near EOF).
    const size_t kBufferPadding = 128;

    inline bool can_pad_input(size_t size) {
        return size <= std::numeric_limits<size_t>::max() - kBufferPadding;
    }

    namespace internal {
#if defined(_WIN32)
        typedef HANDLE InputFile;
        static const InputFile kInvalidInputFile = INVALID_HANDLE_VALUE;

        inline void set_file_error(DWORD error) {
            switch (error) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND: errno = ENOENT; break;
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION: errno = EACCES; break;
            case ERROR_INVALID_NAME:
            case ERROR_INVALID_PARAMETER: errno = EINVAL; break;
            case ERROR_NOT_ENOUGH_MEMORY:
            case ERROR_OUTOFMEMORY: errno = ENOMEM; break;
            default: errno = EIO; break;
            }
        }

        inline InputFile open_input_file(const char* path) {
            const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, 0, 0);
            if (!length) {
                set_file_error(GetLastError());
                return kInvalidInputFile;
            }
            wchar_t* wide = new (std::nothrow) wchar_t[length];
            if (!wide) {
                errno = ENOMEM;
                return kInvalidInputFile;
            }
            if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, length)) {
                set_file_error(GetLastError());
                delete[] wide;
                return kInvalidInputFile;
            }
            InputFile file = CreateFileW(
                wide, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_BACKUP_SEMANTICS, 0);
            delete[] wide;
            if (file == kInvalidInputFile) {
                set_file_error(GetLastError());
                return file;
            }
            BY_HANDLE_FILE_INFORMATION info;
            if (!GetFileInformationByHandle(file, &info)) {
                const DWORD error = GetLastError();
                CloseHandle(file);
                set_file_error(error);
                return kInvalidInputFile;
            }
            if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                CloseHandle(file);
                errno = EISDIR;
                return kInvalidInputFile;
            }
            return file;
        }

        inline void close_input_file(InputFile file) { CloseHandle(file); }

        inline bool input_file_size(InputFile file, u64& size) {
            LARGE_INTEGER value;
            if (!GetFileSizeEx(file, &value)) {
                set_file_error(GetLastError());
                return false;
            }
            if (value.QuadPart < 0) {
                errno = EIO;
                return false;
            }
            size = static_cast<u64>(value.QuadPart);
            return true;
        }

        inline ptrdiff_t read_input_file(InputFile file, char* data, size_t size) {
            const DWORD amount = static_cast<DWORD>((std::min)(size, static_cast<size_t>(0xffffffffu)));
            DWORD read = 0;
            if (!ReadFile(file, data, amount, &read, 0)) {
                set_file_error(GetLastError());
                return -1;
            }
            return static_cast<ptrdiff_t>(read);
        }
#else
        typedef int InputFile;
        static const InputFile kInvalidInputFile = -1;

        inline InputFile open_input_file(const char* path) { return open(path, O_RDONLY); }

        inline void close_input_file(InputFile file) { close(file); }

        inline bool input_file_size(InputFile file, u64& size) {
            struct stat info;
            if (fstat(file, &info) != 0) {
                return false;
            }
            if (info.st_size < 0) {
                errno = EIO;
                return false;
            }
            size = static_cast<u64>(info.st_size);
            return true;
        }

        inline ptrdiff_t read_input_file(InputFile file, char* data, size_t size) {
            return read(file, data, size);
        }
#endif

    } // namespace internal

    // A padded copy of a file or string (malloc'd), for tests, tools and read_into.
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
        nonstd::string_view view() const { return nonstd::string_view(data, size); }

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
        const internal::InputFile file = internal::open_input_file(path);
        if (file == internal::kInvalidInputFile) {
            return false;
        }
        u64 length64 = 0;
        if (!internal::input_file_size(file, length64) ||
            length64 > static_cast<u64>(std::numeric_limits<size_t>::max())) {
            const int error = errno;
            internal::close_input_file(file);
            errno = error ? error : EIO;
            return false;
        }
        const size_t length = static_cast<size_t>(length64);
        if (!buf.reserve(length)) {
            internal::close_input_file(file);
            errno = ENOMEM;
            return false;
        }
        size_t done = 0;
        while (done < length) {
            const ptrdiff_t count = internal::read_input_file(file, buf.data + done, length - done);
            if (count < 0 && errno == EINTR) {
                continue;
            }
            if (count <= 0) {
                const int error = errno;
                internal::close_input_file(file);
                errno = error ? error : EIO;
                return false;
            }
            done += static_cast<size_t>(count);
        }
        const int error = errno;
        internal::close_input_file(file);
        errno = error;
        std::memset(buf.data + length, 0, kBufferPadding);
        buf.size = length;
        return true;
    }

    // Copies an in-memory string into a padded buffer.
    inline bool make_padded(nonstd::string_view content, FileBuffer& buf) {
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
