#ifndef FOSU_ENGINE_PARSING_STRING_LOOKUP_H
#define FOSU_ENGINE_PARSING_STRING_LOOKUP_H

#include <cassert>
#include <cstddef>

#include <fosu/compiler.h>
#include <fosu/span.h>

namespace fosu { namespace internal {

    // FNV-1a over the key bytes.
    inline fosu_uint32 string_hash(StringView key) {
        fosu_uint32 hash = 2166136261u;
        for (size_t i = 0; i < key.size(); i++) {
            hash ^= static_cast<unsigned char>(key[i]);
            hash *= 16777619u;
        }
        return hash;
    }

    template <typename Value>
    struct StringEntry {
        const char* key;
        Value value;
    };

    // Immutable open-addressed table built once from a static entry array (upstream builds it at
    // compile time). Zero slots mark misses; occupied slots store one-based entry indices.
    template <typename Value>
    class StringLookup {
    public:
        static const size_t kMaxEntries = 64;
        static const size_t kCapacity = 256; // >= 2 * kMaxEntries, power of two
        static const size_t kMask = kCapacity - 1;

        StringLookup(const StringEntry<Value>* entries, size_t count)
            : entries_(entries),
              max_key_size_(0) {
            assert(count <= kMaxEntries);
            for (size_t s = 0; s < kCapacity; s++) {
                slots_[s] = 0;
            }
            for (size_t i = 0; i < count; i++) {
                const StringView key(entries[i].key);
                if (key.size() > max_key_size_) {
                    max_key_size_ = key.size();
                }
                hashes_[i] = string_hash(key);
                size_t slot = hashes_[i] & kMask;
                while (slots_[slot]) {
                    slot = (slot + 1) & kMask;
                }
                slots_[slot] = i + 1;
            }
        }

        const Value* find(StringView key) const {
            if (key.size() > max_key_size_) {
                return 0;
            }
            const fosu_uint32 hash = string_hash(key);
            size_t slot = hash & kMask;
            while (slots_[slot]) {
                const size_t index = slots_[slot] - 1;
                // Full equality is required even when the entire hash matches.
                if (hashes_[index] == hash && StringView(entries_[index].key) == key) {
                    return &entries_[index].value;
                }
                slot = (slot + 1) & kMask;
            }
            return 0;
        }

    private:
        const StringEntry<Value>* entries_;
        fosu_uint32 hashes_[kMaxEntries];
        size_t slots_[kCapacity];
        size_t max_key_size_;
    };

}} // namespace fosu::internal

#endif
