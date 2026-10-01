#include "pppp/utils/vector2.h"
#include <cmath>
#include <cstring>

namespace pppp { namespace utils {
    namespace {
        float inverse_sqrt_fast(float x) {
            float xhalf = 0.5f * x;
            int i;
            std::memcpy(&i, &x, sizeof(i));
            i = 0x5f375a86 - (i >> 1);
            std::memcpy(&x, &i, sizeof(x));
            x = x * (1.5f - xhalf * x * x);
            return x;
        }
    } // namespace

    Vector2 operator+(Vector2 a, Vector2 b) {
        Vector2 r = {a.x + b.x, a.y + b.y};
        return r;
    }

    Vector2 operator-(Vector2 a, Vector2 b) {
        Vector2 r = {a.x - b.x, a.y - b.y};
        return r;
    }

    Vector2 operator*(Vector2 a, float s) {
        Vector2 r = {a.x * s, a.y * s};
        return r;
    }

    float length(Vector2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

    float length_squared(Vector2 v) { return v.x * v.x + v.y * v.y; }

    float length_fast(Vector2 v) { return 1.0f / inverse_sqrt_fast(length_squared(v)); }

    float distance(Vector2 a, Vector2 b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float dot(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y; }

    Vector2 normalize(Vector2 v) {
        float len = length(v);
        if (len < 1e-10f) {
            Vector2 r = {0.0f, 0.0f};
            return r;
        }
        Vector2 r = {v.x / len, v.y / len};
        return r;
    }
}} // namespace pppp::utils
