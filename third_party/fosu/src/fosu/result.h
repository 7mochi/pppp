#ifndef FOSU_RESULT_H
#define FOSU_RESULT_H

#include <cassert>
#include <cstddef>

namespace fosu {

    struct ErrorCode {
        enum Value { InvalidInput, IoFailure, InputTooLarge, AllocationFailure };
    };

    const size_t kNoErrorOffset = static_cast<size_t>(-1);

    struct Error {
        ErrorCode::Value code;
        size_t input_offset;
        int os_code;

        Error()
            : code(ErrorCode::InvalidInput),
              input_offset(kNoErrorOffset),
              os_code(0) {}
        explicit Error(ErrorCode::Value c, size_t offset = kNoErrorOffset, int os = 0)
            : code(c),
              input_offset(offset),
              os_code(os) {}
    };

    // A value or an error (no exceptions). `T` must be default constructible.
    template <typename T>
    class Result {
    public:
        Result(const T& value)
            : value_(value),
              error_(),
              succeeded_(true) {}
        Result(const Error& error)
            : value_(),
              error_(error),
              succeeded_(false) {}

        bool ok() const { return succeeded_; }
        bool failed() const { return !succeeded_; }

        T& value() {
            assert(succeeded_);
            return value_;
        }
        const T& value() const {
            assert(succeeded_);
            return value_;
        }
        const Error& error() const {
            assert(!succeeded_);
            return error_;
        }

    private:
        T value_;
        Error error_;
        bool succeeded_;
    };

} // namespace fosu

#endif
