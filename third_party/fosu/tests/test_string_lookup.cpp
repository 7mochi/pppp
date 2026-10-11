// String lookup table (upstream tests/test_string_lookup.cc). The upstream
// cases are compile-time (constexpr/static_assert); the C++98 port checks the
// same properties at run time.
#include "test_support.h"

#include <fosu/engine/parsing/string_lookup.h>

using fosu::internal::string_hash;
using fosu::internal::StringEntry;
using fosu::internal::StringLookup;

namespace {
    // Distinct strings that collide across the full 32-bit FNV-1a hash, not
    // merely the table's bucket mask. Neither may impersonate the other.
    StringEntry<int> collision_entries[] = {{"costarring", 1}, {"liquid", 2}, {"", 3}};

    int handler_one() { return 1; }
    int handler_two() { return 2; }
} // namespace

TEST_CASE("string lookup collisions") {
    CHECK(string_hash("costarring") == string_hash("liquid"));
    StringLookup<int> collisions(collision_entries, 3);
    CHECK_UNARY(collisions.find("costarring") != 0 && *collisions.find("costarring") == 1);
    CHECK_UNARY(collisions.find("liquid") != 0 && *collisions.find("liquid") == 2);
    CHECK_UNARY(collisions.find("") != 0 && *collisions.find("") == 3);
    CHECK_UNARY(collisions.find("Costarring") == 0);
}

TEST_CASE("string lookup runtime misses") {
    StringEntry<int> single_entry[] = {{"costarring", 1}};
    StringLookup<int> single(single_entry, 1);
    CHECK_UNARY(single.find("liquid") == 0);
    StringLookup<int> collisions(collision_entries, 3);
    static const char* const keys[] = {"cost", "costarring-extra", "unknown"};
    for (size_t i = 0; i < 3; i++) {
        CHECK_UNARY(collisions.find(keys[i]) == 0);
    }
    CHECK(*collisions.find("costarring") == 1);
    CHECK(*collisions.find("liquid") == 2);
}

TEST_CASE("string lookup function pointer values") {
    StringEntry<int (*)()> handlers[] = {{"one", &handler_one}, {"two", &handler_two}};
    StringLookup<int (*)()> lookup(handlers, 2);
    CHECK((*lookup.find("two"))() == 2);
}
