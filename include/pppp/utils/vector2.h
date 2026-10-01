#ifndef PPPP_UTILS_VECTOR2_H
#define PPPP_UTILS_VECTOR2_H

namespace pppp { namespace utils {
    struct Vector2 {
        float x, y;
    };

    inline Vector2 vec2(float x, float y) {
        Vector2 v = {x, y};
        return v;
    }

    Vector2 operator+(Vector2 a, Vector2 b);
    Vector2 operator-(Vector2 a, Vector2 b);
    Vector2 operator*(Vector2 a, float s);

    float length(Vector2 v);
    float length_squared(Vector2 v);
    /// Fast inverse-sqrt approximation of the length.
    float length_fast(Vector2 v);
    float distance(Vector2 a, Vector2 b);
    float dot(Vector2 a, Vector2 b);
    Vector2 normalize(Vector2 v);
}} // namespace pppp::utils

#endif
