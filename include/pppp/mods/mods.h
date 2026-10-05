#ifndef PPPP_MODS_MODS_H
#define PPPP_MODS_MODS_H

#include "pppp/mods/mod.h"
#include "pppp/status.h"
#include <cstddef>
#include <vector>

namespace pppp { namespace mods {
    class Mods {
    public:
        typedef std::vector<Mod>::const_iterator const_iterator;
        typedef const_iterator iterator;

        Mods();

        template <class InputIterator>
        Mods(InputIterator first, InputIterator last)
            : list(first, last) {}

        static Mods from_legacy(unsigned bits);

        Status parse(const char* specification);

        void push_back(const Mod& mod);
        void push_back(ModId id);
        void clear();

        bool contains(ModId id) const;
        double clock_rate() const;
        unsigned to_legacy() const;

        bool empty() const;
        size_t size() const;
        const Mod& operator[](size_t index) const;
        const Mod* data() const;
        const_iterator begin() const;
        const_iterator end() const;

    private:
        std::vector<Mod> list;
    };
}} // namespace pppp::mods

#endif
