#include "pppp/mods/mods.h"
#include <cstring>

namespace pppp { namespace mods {
    Mods::Mods()
        : list() {}

    Mods Mods::from_legacy(unsigned bits) {
        Mod converted[32];
        const int count = mod_from_legacy(converted, 32, bits);
        Mods out;
        if (count > 0) {
            out.list.assign(converted, converted + count);
        }
        return out;
    }

    Status Mods::parse(const char* specification) {
        if (!specification) {
            return StatusCode::INVALID_ARGUMENT;
        }
        std::vector<Mod> parsed(std::strlen(specification) / 2 + 1);
        const int count = mod_from_acronyms(&parsed[0], parsed.size(), specification);
        if (count < 0) {
            return StatusCode::INVALID_ARGUMENT;
        }
        parsed.resize(static_cast<size_t>(count));
        list.swap(parsed);
        return StatusCode::OK;
    }

    void Mods::push_back(const Mod& mod) { list.push_back(mod); }

    void Mods::push_back(ModId id) {
        Mod mod;
        mod_make(&mod, id);
        list.push_back(mod);
    }

    void Mods::clear() { list.clear(); }

    bool Mods::contains(ModId id) const { return mod_has(data(), list.size(), id); }

    double Mods::clock_rate() const { return mod_calculate_rate(data(), list.size()); }

    unsigned Mods::to_legacy() const { return mod_to_legacy(data(), list.size()); }

    bool Mods::empty() const { return list.empty(); }

    size_t Mods::size() const { return list.size(); }

    const Mod& Mods::operator[](size_t index) const { return list[index]; }

    const Mod* Mods::data() const { return list.empty() ? 0 : &list[0]; }

    Mods::const_iterator Mods::begin() const { return list.begin(); }

    Mods::const_iterator Mods::end() const { return list.end(); }
}} // namespace pppp::mods
