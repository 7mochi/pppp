#ifndef FOSU_LEGACY_RULES_H
#define FOSU_LEGACY_RULES_H

#include <algorithm>
#include <cstring>

#include <fosu/beatmap.h>

namespace fosu { namespace internal {

    struct HitObjectTimeLess {
        bool operator()(const HitObject& a, const HitObject& b) const { return a.time < b.time; }
    };

    // The parsing engines do not allocate. Only an out-of-order map needs this
    // temporary merge buffer; equal timestamps retain their original input order.
    inline bool sort_hit_objects(Span<HitObject> objects, Arena* scratch_arena) {
        const size_t checkpoint = arena_pos(scratch_arena);
        HitObject* scratch = arena_push_array<HitObject>(scratch_arena, objects.size());
        if (!scratch) {
            return false;
        }
        HitObject* source = objects.data();
        HitObject* destination = scratch;
        for (size_t width = 1; width < objects.size(); width *= 2) {
            for (size_t begin = 0; begin < objects.size(); begin += width * 2) {
                const size_t middle = std::min(begin + width, objects.size());
                const size_t end = std::min(begin + width * 2, objects.size());
                std::merge(source + begin, source + middle, source + middle, source + end,
                           destination + begin, HitObjectTimeLess());
            }
            std::swap(source, destination);
        }
        if (source != objects.data()) {
            std::memcpy(objects.data(), source, objects.size_bytes());
        }
        arena_pop_to(scratch_arena, checkpoint);
        return true;
    }

    inline bool hit_objects_ordered(Span<const HitObject> objects) {
        for (size_t i = 1; i < objects.size(); i++) {
            if (objects[i].time < objects[i - 1].time) {
                return false;
            }
        }
        return true;
    }

    // First object in [first, last) starting strictly after `time` (std::upper_bound).
    inline HitObject* first_object_after(HitObject* first, HitObject* last, double time) {
        size_t count = static_cast<size_t>(last - first);
        while (count > 0) {
            const size_t step = count / 2;
            HitObject* middle = first + step;
            if (!(time < middle->time)) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    inline bool apply_legacy_rules(Beatmap& map, Arena* scratch_arena) {
        const bool ordered = hit_objects_ordered(map.hit_objects);
        if (!ordered && !sort_hit_objects(map.hit_objects, scratch_arena)) {
            return false;
        }

        // Breaks are sparse. Find the first later object without another full walk
        // over hitobjects. Keep the cursor to match osu! even for unordered breaks.
        HitObject* next_object = map.hit_objects.begin();
        for (size_t i = 0; i < map.breaks.size(); i++) {
            next_object = first_object_after(next_object, map.hit_objects.end(), map.breaks[i].end);
            if (next_object == map.hit_objects.end()) {
                break;
            }
            next_object->new_combo = true;
        }
        return true;
    }

}} // namespace fosu::internal

#endif
