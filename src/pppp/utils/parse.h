#ifndef PPPP_UTILS_PARSE_H
#define PPPP_UTILS_PARSE_H

#include <cstring>

namespace pppp { namespace utils {
    inline int parse_bool(const char* text, bool* out) {
        if (text == 0 || out == 0) {
            return -1;
        }
        if (!std::strcmp(text, "1") || !std::strcmp(text, "true") || !std::strcmp(text, "True")) {
            *out = true;
            return 0;
        }
        if (!std::strcmp(text, "0") || !std::strcmp(text, "false") || !std::strcmp(text, "False")) {
            *out = false;
            return 0;
        }
        return -1;
    }
}} // namespace pppp::utils

#endif
