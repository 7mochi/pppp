#ifndef FOSU_SPAN_H
#define FOSU_SPAN_H

#include <cstddef>
#include <cstring>

namespace fosu {

    // A borrowed [pointer, count) view, the C++98 stand-in for std::span. Beatmap arrays are spans
    // into parser-owned storage.
    template <typename T>
    struct Span {
        T* data_;
        size_t size_;

        Span()
            : data_(0),
              size_(0) {}
        Span(T* data, size_t size)
            : data_(data),
              size_(size) {}

        T* data() const { return data_; }
        size_t size() const { return size_; }
        size_t size_bytes() const { return size_ * sizeof(T); }
        bool empty() const { return size_ == 0; }
        T& operator[](size_t i) const { return data_[i]; }
        T& front() const { return data_[0]; }
        T& back() const { return data_[size_ - 1]; }
        T* begin() const { return data_; }
        T* end() const { return data_ + size_; }
        Span first(size_t count) const { return Span(data_, count); }
        Span subspan(size_t offset) const { return Span(data_ + offset, size_ - offset); }
        Span subspan(size_t offset, size_t count) const { return Span(data_ + offset, count); }
        // conversion to a span of const elements
        operator Span<const T>() const { return Span<const T>(data_, size_); }
    };

    // A borrowed string, the C++98 stand-in for std::string_view. Not null-terminated.
    struct StringView {
        const char* data_;
        size_t size_;

        StringView()
            : data_(0),
              size_(0) {}
        StringView(const char* data, size_t size)
            : data_(data),
              size_(size) {}
        StringView(const char* literal)
            : data_(literal),
              size_(std::strlen(literal)) {}

        static const size_t npos = static_cast<size_t>(-1);

        const char* data() const { return data_; }
        size_t size() const { return size_; }
        bool empty() const { return size_ == 0; }
        char operator[](size_t i) const { return data_[i]; }
        char front() const { return data_[0]; }
        char back() const { return data_[size_ - 1]; }
        const char* begin() const { return data_; }
        const char* end() const { return data_ + size_; }
        void remove_prefix(size_t n) {
            data_ += n;
            size_ -= n;
        }
        void remove_suffix(size_t n) { size_ -= n; }
        StringView substr(size_t pos) const { return StringView(data_ + pos, size_ - pos); }
        StringView substr(size_t pos, size_t count) const {
            return StringView(data_ + pos, count < size_ - pos ? count : size_ - pos);
        }
        size_t find(char c, size_t from = 0) const {
            for (size_t i = from; i < size_; i++) {
                if (data_[i] == c) {
                    return i;
                }
            }
            return npos;
        }
        size_t find(StringView needle, size_t from = 0) const {
            if (needle.size_ == 0) {
                return from <= size_ ? from : npos;
            }
            if (needle.size_ > size_) {
                return npos;
            }
            for (size_t i = from; i + needle.size_ <= size_; i++) {
                if (std::memcmp(data_ + i, needle.data_, needle.size_) == 0) {
                    return i;
                }
            }
            return npos;
        }
        bool starts_with(StringView prefix) const {
            return prefix.size_ <= size_ && std::memcmp(data_, prefix.data_, prefix.size_) == 0;
        }
    };

    inline bool operator==(StringView a, StringView b) {
        return a.size() == b.size() && (a.size() == 0 || std::memcmp(a.data(), b.data(), a.size()) == 0);
    }
    inline bool operator!=(StringView a, StringView b) { return !(a == b); }

    // A value that may be absent, the C++98 stand-in for std::optional (T must be default
    // constructible; the absent value is never read).
    template <typename T>
    struct Optional {
        bool present;
        T value;

        Optional()
            : present(false),
              value() {}
        Optional(const T& v)
            : present(true),
              value(v) {}

        bool has_value() const { return present; }
        const T& operator*() const { return value; }
        T& operator*() { return value; }
        const T* operator->() const { return &value; }
        T value_or(const T& fallback) const { return present ? value : fallback; }
    };

    struct NullOptional {};
    inline NullOptional nullopt() { return NullOptional(); }

} // namespace fosu

#endif
