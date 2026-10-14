// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#ifndef PPPP_UTILS_SORT_OSU_LEGACY_H
#define PPPP_UTILS_SORT_OSU_LEGACY_H

#include <cstddef>

namespace pppp { namespace utils { namespace sort {
    namespace legacy_sort_detail {
        const int QUICK_SORT_DEPTH_THRESHOLD = 32;

        template <typename T>
        void swap_at(T* keys, int i, int j) {
            if (i != j) {
                const T tmp = keys[i];
                keys[i] = keys[j];
                keys[j] = tmp;
            }
        }

        template <typename T, typename Compare>
        void swap_if_greater(T* keys, Compare compare, int a, int b) {
            if (a != b && compare(keys[a], keys[b]) > 0) {
                const T tmp = keys[a];
                keys[a] = keys[b];
                keys[b] = tmp;
            }
        }

        template <typename T, typename Compare>
        void down_heap(T* keys, int i, int n, int lo, Compare compare) {
            const T d = keys[lo + i - 1];

            while (i <= n / 2) {
                int child = 2 * i;

                if (child < n && compare(keys[lo + child - 1], keys[lo + child]) < 0) {
                    child++;
                }
                if (!(compare(d, keys[lo + child - 1]) < 0)) {
                    break;
                }

                keys[lo + i - 1] = keys[lo + child - 1];
                i = child;
            }

            keys[lo + i - 1] = d;
        }

        template <typename T, typename Compare>
        void heapsort(T* keys, int lo, int hi, Compare compare) {
            const int n = hi - lo + 1;

            for (int i = n / 2; i >= 1; i--) {
                down_heap(keys, i, n, lo, compare);
            }
            for (int i = n; i > 1; i--) {
                swap_at(keys, lo, lo + i - 1);
                down_heap(keys, 1, i - 1, lo, compare);
            }
        }

        template <typename T, typename Compare>
        void depth_limited_quick_sort(T* keys, int left, int right, Compare compare, int depth_limit) {
            do {
                if (depth_limit == 0) {
                    heapsort(keys, left, right, compare);
                    return;
                }

                int i = left;
                int j = right;

                // pre-sort the low, middle (pivot), and high values in place.
                // this improves performance in the face of already sorted data, or
                // data that is made up of multiple sorted runs appended together.
                const int middle = i + ((j - i) >> 1);
                swap_if_greater(keys, compare, i, middle); // swap the low with the mid point
                swap_if_greater(keys, compare, i, j); // swap the low with the high
                swap_if_greater(keys, compare, middle, j); // swap the middle with the high

                const T x = keys[middle];

                do {
                    while (compare(keys[i], x) < 0) {
                        i++;
                    }
                    while (compare(x, keys[j]) < 0) {
                        j--;
                    }
                    if (i > j) {
                        break;
                    }
                    if (i < j) {
                        const T tmp = keys[i];
                        keys[i] = keys[j];
                        keys[j] = tmp;
                    }
                    i++;
                    j--;
                } while (i <= j);

                // The next iteration of the while loop is to "recursively" sort the larger half of the array
                // and the following calls recrusively sort the smaller half.  So we subtrack one from
                // depthLimit here so both sorts see the new value.
                depth_limit--;

                if (j - left <= right - i) {
                    if (left < j) {
                        depth_limited_quick_sort(keys, left, j, compare, depth_limit);
                    }
                    left = i;
                } else {
                    if (i < right) {
                        depth_limited_quick_sort(keys, i, right, compare, depth_limit);
                    }
                    right = j;
                }
            } while (left < right);
        }
    } // namespace legacy_sort_detail

    /// Provides access to .NET4.0 unstable sorting methods.
    /// @remarks Source:
    /// https://referencesource.microsoft.com/#mscorlib/system/collections/generic/arraysorthelper.cs
    /// Copyright (c) Microsoft Corporation.  All rights reserved.
    template <typename T, typename Compare>
    void legacy_sort(T* keys, size_t count, Compare compare) {
        if (count == 0) {
            return;
        }
        legacy_sort_detail::depth_limited_quick_sort(keys, 0, static_cast<int>(count) - 1, compare,
                                                     legacy_sort_detail::QUICK_SORT_DEPTH_THRESHOLD);
    }
}}} // namespace pppp::utils::sort

#endif
