#ifndef PPPP_UTILS_SORT_CSHARP_H
#define PPPP_UTILS_SORT_CSHARP_H

#include <vector>

namespace pppp { namespace utils { namespace sort {
    namespace introsort_detail {
        const int INTROSORT_SIZE_THRESHOLD = 16;

        inline int log2(unsigned value) {
            int result = 0;
            while (value >>= 1) {
                result++;
            }
            return result;
        }

        template <typename T, typename Comparison>
        void swap_if_greater(T* keys, Comparison comparer, int i, int j) {
            if (comparer(keys[i], keys[j]) > 0) {
                T key = keys[i];
                keys[i] = keys[j];
                keys[j] = key;
            }
        }

        template <typename T>
        void swap(T* a, int i, int j) {
            T t = a[i];
            a[i] = a[j];
            a[j] = t;
        }

        template <typename T, typename Comparison>
        void insertion_sort(T* keys, int length, Comparison comparer) {
            for (int i = 0; i < length - 1; i++) {
                T t = keys[i + 1];

                int j = i;
                while (j >= 0 && comparer(t, keys[j]) < 0) {
                    keys[j + 1] = keys[j];
                    j--;
                }

                keys[j + 1] = t;
            }
        }

        template <typename T, typename Comparison>
        void down_heap(T* keys, int i, int n, Comparison comparer) {
            T d = keys[i - 1];
            while (i <= n >> 1) {
                int child = 2 * i;
                if (child < n && comparer(keys[child - 1], keys[child]) < 0) {
                    child++;
                }

                if (!(comparer(d, keys[child - 1]) < 0)) {
                    break;
                }

                keys[i - 1] = keys[child - 1];
                i = child;
            }

            keys[i - 1] = d;
        }

        template <typename T, typename Comparison>
        void heap_sort(T* keys, int n, Comparison comparer) {
            for (int i = n >> 1; i >= 1; i--) {
                down_heap(keys, i, n, comparer);
            }

            for (int i = n; i > 1; i--) {
                swap(keys, 0, i - 1);
                down_heap(keys, 1, i - 1, comparer);
            }
        }

        template <typename T, typename Comparison>
        int pick_pivot_and_partition(T* keys, int length, Comparison comparer) {
            int hi = length - 1;

            // Compute median-of-three.  But also partition them, since we've done the comparison.
            int middle = hi >> 1;

            // Sort lo, mid and hi appropriately, then pick mid as the pivot.
            swap_if_greater(keys, comparer, 0, middle); // swap the low with the mid point
            swap_if_greater(keys, comparer, 0, hi); // swap the low with the high
            swap_if_greater(keys, comparer, middle, hi); // swap the middle with the high

            T pivot = keys[middle];
            swap(keys, middle, hi - 1);
            int left = 0, right = hi - 1; // We already partitioned lo and hi and put the pivot in hi - 1. And
                                          // we pre-increment & decrement below.

            while (left < right) {
                while (comparer(keys[++left], pivot) < 0) {}
                while (comparer(pivot, keys[--right]) < 0) {}

                if (left >= right) {
                    break;
                }

                swap(keys, left, right);
            }

            // Put pivot in the right location.
            if (left != hi - 1) {
                swap(keys, left, hi - 1);
            }
            return left;
        }

        template <typename T, typename Comparison>
        void intro_sort(T* keys, int length, int depth_limit, Comparison comparer) {
            int partition_size = length;
            while (partition_size > 1) {
                if (partition_size <= INTROSORT_SIZE_THRESHOLD) {
                    if (partition_size == 2) {
                        swap_if_greater(keys, comparer, 0, 1);
                        return;
                    }

                    if (partition_size == 3) {
                        swap_if_greater(keys, comparer, 0, 1);
                        swap_if_greater(keys, comparer, 0, 2);
                        swap_if_greater(keys, comparer, 1, 2);
                        return;
                    }

                    insertion_sort(keys, partition_size, comparer);
                    return;
                }

                if (depth_limit == 0) {
                    heap_sort(keys, partition_size, comparer);
                    return;
                }
                depth_limit--;

                int p = pick_pivot_and_partition(keys, partition_size, comparer);

                // Note we've already partitioned around the pivot and do not have to move the pivot again.
                intro_sort(keys + (p + 1), partition_size - (p + 1), depth_limit, comparer);
                partition_size = p;
            }
        }
    } // namespace introsort_detail

    /// `List<T>.Sort(Comparison<T>)` as .NET 8 runs it, an unstable introspective sort:
    /// https://github.com/dotnet/runtime/blob/26d2fafe1c84107e03ccdd8d6d8974ee7b49d9a5/src/libraries/System.Private.CoreLib/src/System/Collections/Generic/ArraySortHelper.cs#L121
    /// @param keys The list to sort in place.
    /// @param comparer The comparison, returning a negative number, zero or a positive number.
    template <typename T, typename Comparison>
    void introspective_sort(std::vector<T>& keys, Comparison comparer) {
        if (keys.size() > 1) {
            const int length = static_cast<int>(keys.size());
            introsort_detail::intro_sort(
                &keys[0], length, 2 * (introsort_detail::log2(static_cast<unsigned>(length)) + 1), comparer);
        }
    }
}}} // namespace pppp::utils::sort

#endif
