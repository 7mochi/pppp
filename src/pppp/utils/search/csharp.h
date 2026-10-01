#ifndef PPPP_UTILS_SEARCH_CSHARP_H
#define PPPP_UTILS_SEARCH_CSHARP_H

#include "pppp/utils/math/csharp.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace utils { namespace search {
    namespace binary_search_detail {
        inline int compare_to(double element, double value) { return math::compare_to(element, value); }

        template <typename T>
        int compare_to(const T& element, const T& value) {
            return element.compare_to(value);
        }
    } // namespace binary_search_detail

    /// `List<T>.BinarySearch` as .NET 8 runs it:
    /// https://github.com/dotnet/runtime/blob/26d2fafe1c84107e03ccdd8d6d8974ee7b49d9a5/src/libraries/System.Private.CoreLib/src/System/Collections/Generic/ArraySortHelper.cs#L352
    /// Each step orders an element against `value` with the element's `CompareTo` (`double.CompareTo`
    /// for doubles, a `compare_to` member otherwise). The range is halved at `lo + ((hi - lo) >> 1)`
    /// and the first exact match it lands on is returned, so in a run of equal elements the index is
    /// wherever the halving meets the run, not its first or its last element. `std::lower_bound` and
    /// `std::upper_bound` pick a different one.
    /// @param list The list, in `CompareTo` order.
    /// @param value The value to look for.
    /// @returns The index of a matching element, or the bitwise complement of the index where
    /// `value` would be inserted.
    template <typename T>
    int binary_search(const std::vector<T>& list, const T& value) {
        int lo = 0;
        int hi = static_cast<int>(list.size()) - 1;
        while (lo <= hi) {
            const int i = lo + ((hi - lo) >> 1);
            const int order = binary_search_detail::compare_to(list[static_cast<size_t>(i)], value);
            if (order == 0) {
                return i;
            }
            if (order < 0) {
                lo = i + 1;
            } else {
                hi = i - 1;
            }
        }
        return ~lo;
    }
}}} // namespace pppp::utils::search

#endif
