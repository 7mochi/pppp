#ifndef PPPP_STATUS_H
#define PPPP_STATUS_H

namespace pppp {
    struct StatusCode {
        enum Value { OK = 0, INVALID_ARGUMENT, ALLOCATION, PARSE, NO_SLIDER_PATH_BACKEND, SLIDER_PATH };
    };

    class Status {
    public:
        Status(StatusCode::Value code = StatusCode::OK);

        bool ok() const;
        StatusCode::Value code() const;
        const char* message() const;

    private:
        StatusCode::Value value;
    };

    inline Status::Status(StatusCode::Value code)
        : value(code) {}

    inline bool Status::ok() const { return value == StatusCode::OK; }

    inline StatusCode::Value Status::code() const { return value; }

    inline const char* Status::message() const {
        switch (value) {
        case StatusCode::OK: return "no error";
        case StatusCode::INVALID_ARGUMENT: return "invalid argument";
        case StatusCode::ALLOCATION: return "out of memory";
        case StatusCode::PARSE: return "cannot parse the beatmap";
        case StatusCode::NO_SLIDER_PATH_BACKEND: return "no slider path backend";
        case StatusCode::SLIDER_PATH: return "a slider path could not be computed";
        default: return "unknown error";
        }
    }
} // namespace pppp

#endif
