// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_MANIA_PATTERNS_PATTERN_H
#define PPPP_MANIA_PATTERNS_PATTERN_H

#include "pppp/mania/object/mania_hit_object.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mania { namespace patterns {
    /// Creates a pattern containing hit objects.
    class Pattern {
    public:
        std::vector<ManiaHitObject> hit_objects;
        std::vector<bool> contained_columns;

        /// Amount of columns taken up by hit objects in this pattern.
        int column_with_objects;

        Pattern()
            : column_with_objects(0) {}

        /// Check whether a column of this pattern contains a hit object.
        /// @param column The column index.
        /// @returns Whether the column with index @p column contains a hit object.
        bool column_has_object(int column) const {
            return column >= 0 && static_cast<size_t>(column) < contained_columns.size() &&
                   contained_columns[column];
        }

        /// Adds a hit object to this pattern.
        /// @param hit_object The hit object to add.
        void add(const ManiaHitObject& hit_object) {
            hit_objects.push_back(hit_object);

            if (hit_object.column < 0) {
                return;
            }
            if (static_cast<size_t>(hit_object.column) >= contained_columns.size()) {
                contained_columns.resize(static_cast<size_t>(hit_object.column) + 1, false);
            }
            if (!contained_columns[hit_object.column]) {
                contained_columns[hit_object.column] = true;
                column_with_objects++;
            }
        }

        /// Copies hit object from another pattern to this one.
        /// @param other The other pattern.
        void add(const Pattern& other) {
            for (size_t i = 0; i < other.hit_objects.size(); i++) {
                add(other.hit_objects[i]);
            }
        }

        /// Clears this pattern, removing all hit objects.
        void clear() {
            hit_objects.clear();
            contained_columns.assign(contained_columns.size(), false);
            column_with_objects = 0;
        }
    };
}}} // namespace pppp::mania::patterns

#endif
