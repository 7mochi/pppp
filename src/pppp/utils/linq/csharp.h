#ifndef PPPP_UTILS_LINQ_CSHARP_H
#define PPPP_UTILS_LINQ_CSHARP_H

#include "pppp/utils/linq/lambda.h"
#include "pppp/utils/math/csharp.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace pppp { namespace utils { namespace linq {
    namespace detail {
        inline bool equals(double value, double obj) {
            if (obj == value) {
                return true;
            }
            return obj != obj && value != value;
        }

        template <typename T>
        bool equals(const T& value, const T& obj) {
            return value == obj;
        }

        inline int compare(double x, double y) { return math::compare_to(x, y); }

        template <typename T>
        int compare(const T& x, const T& y) {
            return x < y ? -1 : (y < x ? 1 : 0);
        }

        template <typename Key, bool Descending>
        struct KeyOrder {
            Key key;

            template <typename T>
            bool operator()(const T& x, const T& y) const {
                return Descending ? compare(key(y), key(x)) < 0 : compare(key(x), key(y)) < 0;
            }
        };

        template <typename Key, bool Descending>
        struct PlainKeyOrder {
            Key key;

            template <typename T>
            bool operator()(const T& x, const T& y) const {
                return Descending ? key(y) < key(x) : key(x) < key(y);
            }
        };

        inline bool is_nan(double x) { return x != x; }

        template <typename K>
        bool is_nan(const K&) {
            return false;
        }

        template <typename K, bool Descending>
        struct IndexOrder {
            const std::vector<K>* keys;

            bool operator()(size_t x, size_t y) const {
                return Descending ? compare((*keys)[y], (*keys)[x]) < 0 : compare((*keys)[x], (*keys)[y]) < 0;
            }
        };

        template <typename Iterator, typename Key, bool Descending>
        void sort(Iterator begin, Iterator end, const PlainKeyOrder<Key, Descending>& order) {
            std::stable_sort(begin, end, order);
        }

        template <bool Descending>
        void sort(std::vector<double>::iterator begin, std::vector<double>::iterator end,
                  const PlainKeyOrder<Lambda<Argument>, Descending>& order) {
            bool positive_zero = false;
            bool negative_zero = false;
            for (std::vector<double>::iterator it = begin; it != end; ++it) {
                if (*it == 0.0) {
                    (1.0 / *it < 0.0 ? negative_zero : positive_zero) = true;
                }
            }

            if (positive_zero && negative_zero) {
                std::stable_sort(begin, end, order);
            } else {
                std::sort(begin, end, order);
            }
        }

        template <bool Pure>
        struct Sorter {
            template <typename T, typename Key, bool Descending>
            static void apply(std::vector<T>& items, size_t first, size_t last, const Key& key) {
                bool nan = false;
                for (size_t i = first; i < last && !nan; i++) {
                    nan = is_nan(key(items[i]));
                }

                if (nan) {
                    KeyOrder<Key, Descending> order = {key};
                    std::stable_sort(items.begin() + static_cast<std::ptrdiff_t>(first),
                                     items.begin() + static_cast<std::ptrdiff_t>(last), order);
                    return;
                }

                PlainKeyOrder<Key, Descending> order = {key};
                sort(items.begin() + static_cast<std::ptrdiff_t>(first),
                     items.begin() + static_cast<std::ptrdiff_t>(last), order);
            }
        };

        template <>
        struct Sorter<false> {
            template <typename T, typename Key, bool Descending>
            static void apply(std::vector<T>& items, size_t first, size_t last, const Key& key) {
                typedef typename Key::template Result<T>::type K;
                std::vector<K> keys;
                std::vector<size_t> map;
                keys.reserve(last - first);
                map.reserve(last - first);
                for (size_t i = first; i < last; i++) {
                    keys.push_back(key(items[i]));
                    map.push_back(i - first);
                }

                IndexOrder<K, Descending> order = {&keys};
                std::stable_sort(map.begin(), map.end(), order);

                std::vector<T> sorted;
                sorted.reserve(last - first);
                for (size_t i = 0; i < map.size(); i++) {
                    sorted.push_back(items[first + map[i]]);
                }
                std::copy(sorted.begin(), sorted.end(), items.begin() + static_cast<std::ptrdiff_t>(first));
            }
        };
    } // namespace detail

    template <typename K, typename T>
    struct Grouping {
        K key;
        std::vector<T> elements;
    };

    /// `System.Linq.Enumerable` as .NET 8 runs it, each operator from the file of its name in
    /// https://github.com/dotnet/runtime/tree/e3264d80f8eb87573a30e075ee4283365d833bcc/src/libraries/System.Linq/src/System/Linq
    /// The `OrderBy` family computes its keys once and breaks ties by index, as
    /// https://github.com/dotnet/runtime/blob/e3264d80f8eb87573a30e075ee4283365d833bcc/src/libraries/System.Linq/src/System/Linq/OrderedEnumerable.cs#L364
    /// https://github.com/dotnet/runtime/blob/e3264d80f8eb87573a30e075ee4283365d833bcc/src/libraries/System.Linq/src/System/Linq/OrderedEnumerable.cs#L389
    /// do, and `double` keys and elements are ordered and matched with `double.CompareTo` and
    /// `double.Equals`:
    /// https://github.com/dotnet/runtime/blob/26d2fafe1c84107e03ccdd8d6d8974ee7b49d9a5/src/libraries/System.Private.CoreLib/src/System/Double.cs#L258
    /// https://github.com/dotnet/runtime/blob/26d2fafe1c84107e03ccdd8d6d8974ee7b49d9a5/src/libraries/System.Private.CoreLib/src/System/Double.cs#L312
    template <typename T>
    class Enumerable {
    public:
        Enumerable(const T* data_in, size_t count)
            : data(data_in),
              start(0),
              stop(count),
              owned(false),
              is_list(true) {}

        Enumerable(const Enumerable& other)
            : data(other.data),
              start(other.start),
              stop(other.stop),
              owned(other.owned),
              is_list(other.is_list) {
            if (owned) {
                items = other.items;
                data = items.empty() ? 0 : &items[0];
            }
        }

        template <typename Pred>
        Enumerable where(Pred pred) const {
            Enumerable sequence;
            for (size_t i = start; i < stop; i++) {
                if (pred(data[i])) {
                    sequence.items.push_back(data[i]);
                }
            }
            sequence.own();
            return sequence;
        }

        template <typename Sel>
        Enumerable<typename Sel::template Result<T>::type> select(Sel sel) const {
            Enumerable<typename Sel::template Result<T>::type> sequence;
            sequence.items.reserve(stop - start);
            for (size_t i = start; i < stop; i++) {
                sequence.items.push_back(sel(data[i]));
            }
            sequence.own();
            return sequence;
        }

        Enumerable skip(int count) const {
            Enumerable sequence(*this);
            if (count > 0) {
                sequence.start = std::min(stop, start + static_cast<size_t>(count));
            }
            sequence.is_list = false;
            return sequence;
        }

        Enumerable take(int count) const {
            Enumerable sequence(*this);
            sequence.stop = count <= 0 ? start : std::min(stop, start + static_cast<size_t>(count));
            sequence.is_list = false;
            return sequence;
        }

        Enumerable skip_last(int count) const {
            Enumerable sequence(*this);
            if (count > 0) {
                sequence.stop = stop - std::min(stop - start, static_cast<size_t>(count));
            }
            sequence.is_list = false;
            return sequence;
        }

        Enumerable take_last(int count) const {
            Enumerable sequence(*this);
            sequence.start = count <= 0 ? stop : stop - std::min(stop - start, static_cast<size_t>(count));
            sequence.is_list = false;
            return sequence;
        }

        template <typename Key>
        Enumerable<Grouping<typename Key::template Result<T>::type, T> > group_by(Key key) const {
            typedef typename Key::template Result<T>::type K;
            Enumerable<Grouping<K, T> > sequence;
            for (size_t i = start; i < stop; i++) {
                K element_key = key(data[i]);
                size_t g = 0;
                while (g < sequence.items.size() && !detail::equals(sequence.items[g].key, element_key)) {
                    g++;
                }
                if (g == sequence.items.size()) {
                    Grouping<K, T> grouping;
                    grouping.key = element_key;
                    sequence.items.push_back(grouping);
                }
                sequence.items[g].elements.push_back(data[i]);
            }
            sequence.own();
            return sequence;
        }

        Enumerable append(const T& element) const {
            Enumerable sequence;
            sequence.items.reserve(stop - start + 1);
            sequence.items.assign(data + start, data + stop);
            sequence.items.push_back(element);
            sequence.own();
            return sequence;
        }

        Enumerable concat(const std::vector<T>& second) const {
            Enumerable sequence;
            sequence.items.reserve(stop - start + second.size());
            sequence.items.assign(data + start, data + stop);
            sequence.items.insert(sequence.items.end(), second.begin(), second.end());
            sequence.own();
            return sequence;
        }

        Enumerable distinct() const {
            Enumerable sequence;
            sequence.items.reserve(stop - start);
            for (size_t i = start; i < stop; i++) {
                bool seen = false;
                for (size_t j = 0; j < sequence.items.size() && !seen; j++) {
                    seen = detail::equals(sequence.items[j], data[i]);
                }
                if (!seen) {
                    sequence.items.push_back(data[i]);
                }
            }
            sequence.own();
            return sequence;
        }

        template <typename Key>
        Enumerable order_by(Key key) const {
            return sorted<Key, false>(key);
        }

        template <typename Key>
        Enumerable order_by_descending(Key key) const {
            return sorted<Key, true>(key);
        }

        Enumerable order_descending() const { return sorted<Lambda<detail::Argument>, true>(_1); }

        std::vector<T> to_list() const {
            std::vector<T> list;
            if (stop > start) {
                list.assign(data + start, data + stop);
            }
            return list;
        }

        bool any() const { return stop > start; }

        template <typename Pred>
        bool any(Pred pred) const {
            for (size_t i = start; i < stop; i++) {
                if (pred(data[i])) {
                    return true;
                }
            }
            return false;
        }

        int count() const { return static_cast<int>(stop - start); }

        template <typename Pred>
        int count(Pred pred) const {
            int count = 0;
            for (size_t i = start; i < stop; i++) {
                if (pred(data[i])) {
                    count++;
                }
            }
            return count;
        }

        const T& element_at(int index) const {
            static const T empty = T();
            return index >= 0 && static_cast<size_t>(index) < stop - start
                       ? data[start + static_cast<size_t>(index)]
                       : empty;
        }

        T element_at_or_default(int index) const {
            return index >= 0 && static_cast<size_t>(index) < stop - start
                       ? data[start + static_cast<size_t>(index)]
                       : T();
        }

        const T& first() const {
            static const T empty = T();
            return stop > start ? data[start] : empty;
        }

        template <typename Pred>
        const T& first(Pred pred) const {
            static const T empty = T();
            for (size_t i = start; i < stop; i++) {
                if (pred(data[i])) {
                    return data[i];
                }
            }
            return empty;
        }

        const T& last() const {
            static const T empty = T();
            return stop > start ? data[stop - 1] : empty;
        }

        T first_or_default() const { return stop > start ? data[start] : T(); }

        T last_or_default() const { return stop > start ? data[stop - 1] : T(); }

        T sum() const {
            T sum = T();
            for (size_t i = start; i < stop; i++) {
                sum += data[i];
            }
            return sum;
        }

        template <typename Sel>
        typename Sel::template Result<T>::type sum(Sel selector) const {
            typename Sel::template Result<T>::type sum = typename Sel::template Result<T>::type();
            for (size_t i = start; i < stop; i++) {
                sum += selector(data[i]);
            }
            return sum;
        }

        double average() const {
            if (stop == start) {
                return 0.0;
            }
            if (is_list) {
                return sum() / static_cast<double>(stop - start);
            }

            double sum = data[start];
            long count = 1;
            for (size_t i = start + 1; i < stop; i++) {
                sum += data[i];
                count++;
            }

            return sum / static_cast<double>(count);
        }

        T max() const {
            if (stop == start) {
                return T();
            }

            size_t i = start;
            while (i < stop && data[i] != data[i]) {
                i++;
            }

            if (i == stop) {
                return data[stop - 1];
            }

            T value = data[i];
            for (; i < stop; i++) {
                if (data[i] > value) {
                    value = data[i];
                }
            }

            return value;
        }

        template <typename Sel>
        typename Sel::template Result<T>::type max(Sel selector) const {
            typedef typename Sel::template Result<T>::type R;
            if (stop == start) {
                return R();
            }

            size_t i = start;
            R value = selector(data[i]);
            while (value != value) {
                if (++i == stop) {
                    return value;
                }

                value = selector(data[i]);
            }

            while (++i < stop) {
                R x = selector(data[i]);
                if (x > value) {
                    value = x;
                }
            }

            return value;
        }

        template <typename Key>
        T min_by(Key key) const {
            if (stop == start) {
                return T();
            }

            T value = data[start];
            typename Key::template Result<T>::type min_key = key(value);
            for (size_t i = start + 1; i < stop; i++) {
                typename Key::template Result<T>::type next_key = key(data[i]);
                if (detail::compare(next_key, min_key) < 0) {
                    min_key = next_key;
                    value = data[i];
                }
            }

            return value;
        }

    private:
        template <typename U>
        friend class Enumerable;

        friend Enumerable<int> range(int start, int count);

        Enumerable()
            : data(0),
              start(0),
              stop(0),
              owned(true),
              is_list(false) {}

        Enumerable& operator=(const Enumerable&);

        void own() {
            data = items.empty() ? 0 : &items[0];
            start = 0;
            stop = items.size();
        }

        template <typename Key, bool Descending>
        Enumerable sorted(Key key) const {
            Enumerable sequence(*this);
            if (!sequence.owned) {
                sequence.items.assign(data + start, data + stop);
                sequence.owned = true;
                sequence.own();
            }
            detail::Sorter<detail::IsPure<Key>::value>::template apply<T, Key, Descending>(
                sequence.items, sequence.start, sequence.stop, key);
            sequence.is_list = false;
            return sequence;
        }

        mutable std::vector<T> items;
        const T* data;
        size_t start;
        size_t stop;
        mutable bool owned;
        bool is_list;
    };

    template <typename T>
    Enumerable<T> from(const std::vector<T>& list) {
        return Enumerable<T>(list.empty() ? 0 : &list[0], list.size());
    }

    template <typename T, size_t N>
    Enumerable<T> from(const T (&array)[N]) {
        return Enumerable<T>(array, N);
    }

    template <typename T>
    Enumerable<T> from(const T* array, size_t count) {
        return Enumerable<T>(array, count);
    }

    inline Enumerable<int> range(int start, int count) {
        Enumerable<int> sequence;
        sequence.items.reserve(static_cast<size_t>(count > 0 ? count : 0));
        for (int i = 0; i < count; i++) {
            sequence.items.push_back(start + i);
        }
        sequence.own();
        return sequence;
    }
}}} // namespace pppp::utils::linq

#endif
