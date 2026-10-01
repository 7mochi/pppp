#ifndef FOSU_ARENA_H
#define FOSU_ARENA_H

// Bump allocator with the same API as upstream (arena_alloc / arena_push / arena_pos /
// arena_pop_to / arena_clear / arena_release / TempArena), backed by malloc'd blocks chained
// on demand instead of a lazily committed address-space reservation. Positions are monotonic
// across blocks so arena_pos/arena_pop_to keep their meaning.

#include <cstddef>
#include <cstdlib>
#include <limits>

#include <fosu/compiler.h>

namespace fosu {

    const size_t kCacheLineSize = 64;
    const size_t kDefaultArenaBlockSize = static_cast<size_t>(4) << 20;
    const size_t kMaxArenaPush = static_cast<size_t>(1) << 30;
    const size_t kMaxArenaAlignment = static_cast<size_t>(1) << 12;

    enum ArenaFlags { ArenaFlagChain = 1u << 0 };

    struct Arena {
        Arena* prev; // previous block of the chain (null for the first)
        Arena* current; // meaningful on the first block only: the block being filled
        size_t base_pos; // logical position of this block's start
        size_t capacity; // usable bytes after the header
        size_t pos; // next free byte, relative to the block start (>= kArenaHeaderSize)
        fosu_uint32 flags;
    };

    struct ArenaParams {
        size_t block_size;
        fosu_uint32 flags;
    };

    inline size_t align_up(size_t value, size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    const size_t kArenaHeaderSize = ((sizeof(Arena) + kCacheLineSize - 1) / kCacheLineSize) * kCacheLineSize;

    inline Arena* arena_alloc(ArenaParams params) {
        if (params.block_size > std::numeric_limits<size_t>::max() - kArenaHeaderSize) {
            return 0;
        }
        void* memory = std::malloc(kArenaHeaderSize + params.block_size);
        if (!memory) {
            return 0;
        }
        Arena* arena = static_cast<Arena*>(memory);
        arena->prev = 0;
        arena->current = arena;
        arena->base_pos = 0;
        arena->capacity = params.block_size;
        arena->pos = kArenaHeaderSize;
        arena->flags = params.flags;
        return arena;
    }

    inline Arena* arena_alloc() {
        ArenaParams params;
        params.block_size = kDefaultArenaBlockSize;
        params.flags = ArenaFlagChain;
        return arena_alloc(params);
    }

    inline size_t arena_pos(const Arena* arena) {
        if (!arena) {
            return 0;
        }
        return arena->current->base_pos + arena->current->pos;
    }

    inline void* arena_push(Arena* arena, size_t size, size_t alignment) {
        if (!arena || size > kMaxArenaPush || alignment == 0 || alignment > kMaxArenaAlignment ||
            (alignment & (alignment - 1)) != 0 || size > std::numeric_limits<size_t>::max() - alignment) {
            return 0;
        }

        Arena* current = arena->current;
        size_t pos = align_up(current->pos, alignment);
        const size_t limit = kArenaHeaderSize + current->capacity;
        if (pos > limit || size > limit - pos) {
            if (!(current->flags & ArenaFlagChain)) {
                return 0;
            }

            ArenaParams params;
            params.block_size = current->capacity;
            if (params.block_size < alignment + size) {
                params.block_size = alignment + size;
            }
            params.flags = current->flags;
            Arena* block = arena_alloc(params);
            if (!block) {
                return 0;
            }
            block->prev = current;
            block->base_pos = current->base_pos + kArenaHeaderSize + current->capacity;
            arena->current = current = block;
            pos = align_up(current->pos, alignment);
        }

        void* result = reinterpret_cast<char*>(current) + pos;
        current->pos = pos + size;
        return result;
    }

    // alignof(T) without C++11: sizeof is a multiple of the alignment, so the padding a leading
    // char forces in front of a T is exactly the alignment.
    template <typename T>
    struct AlignmentOf {
        struct Probe {
            char c;
            T t;
        };
        enum { value = sizeof(Probe) - sizeof(T) };
    };

    template <typename T>
    inline T* arena_push_array(Arena* arena, size_t count) {
        if (count > std::numeric_limits<size_t>::max() / sizeof(T)) {
            return 0;
        }
        return static_cast<T*>(arena_push(arena, sizeof(T) * count, AlignmentOf<T>::value));
    }

    inline void arena_release(Arena* arena) {
        if (!arena) {
            return;
        }
        for (Arena* block = arena->current; block;) {
            Arena* prev = block->prev;
            std::free(block);
            block = prev;
        }
    }

    inline void arena_pop_to(Arena* arena, size_t pos) {
        if (!arena) {
            return;
        }
        const size_t target = pos > kArenaHeaderSize ? pos : kArenaHeaderSize;
        Arena* current = arena->current;
        while (current->prev && current->base_pos >= target) {
            Arena* prev = current->prev;
            std::free(current);
            current = prev;
        }
        arena->current = current;
        size_t new_pos = target - current->base_pos;
        if (new_pos < kArenaHeaderSize) {
            new_pos = kArenaHeaderSize;
        }
        if (new_pos > current->pos) {
            new_pos = current->pos;
        }
        current->pos = new_pos;
    }

    inline void arena_clear(Arena* arena) { arena_pop_to(arena, kArenaHeaderSize); }

    // Restores the arena position on scope exit.
    class TempArena {
    public:
        explicit TempArena(Arena* arena)
            : arena_(arena),
              pos_(arena_pos(arena)) {}
        ~TempArena() { arena_pop_to(arena_, pos_); }
        size_t position() const { return pos_; }

    private:
        TempArena(const TempArena&);
        TempArena& operator=(const TempArena&);

        Arena* arena_;
        size_t pos_;
    };

} // namespace fosu

#endif
