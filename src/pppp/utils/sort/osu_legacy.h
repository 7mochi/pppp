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

                // Pre-sorting low, middle and high in place is what keeps already sorted runs fast.
                const int middle = i + ((j - i) >> 1);
                swap_if_greater(keys, compare, i, middle);
                swap_if_greater(keys, compare, i, j);
                swap_if_greater(keys, compare, middle, j);

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

                // The larger half is sorted by the next turn of this loop and the smaller half by
                // the recursive call, so both see the decremented limit.
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

    /// Sorts `count` keys with the legacy order the osu!mania difficulty calculation depends on: the
    /// exact order equal-time notes come out in decides which column each note is charged to. This is
    /// the depth-limited quicksort the game's converter used, which is deliberately not stable:
    /// `std::sort` is a different introsort and `std::stable_sort` keeps the input order, so neither
    /// reproduces the permutation the game produced.
    /// @param keys The keys to sort.
    /// @param count How many keys are in `keys`.
    /// @param compare The three-way comparison of two keys; negative, zero or positive.
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
