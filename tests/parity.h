#ifndef PPPP_TESTS_PARITY_H
#define PPPP_TESTS_PARITY_H

#include <cJSON.h>
#include <doctest.h>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>

namespace parity {
    inline const cJSON* root() {
        static cJSON* json = NULL;

        if (json == NULL) {
            std::ifstream file(PPPP_TEST_PARITY, std::ios::in | std::ios::binary);
            const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            json = cJSON_ParseWithLength(text.c_str(), text.size());
            if (json == NULL) {
                FAIL("cannot read " << PPPP_TEST_PARITY);
            }
        }
        return json;
    }

    inline const cJSON* checked(const cJSON* value, bool matches, const char* name) {
        if (!matches) {
            FAIL(PPPP_TEST_PARITY << ": " << name << " is missing or has another type");
        }
        return value;
    }

    inline const cJSON* array(const cJSON* object, const char* name) {
        const cJSON* value = cJSON_GetObjectItemCaseSensitive(object, name);
        return checked(value, cJSON_IsArray(value) != 0, name);
    }

    class Row {
    public:
        explicit Row(const cJSON* item)
            : node(item) {}

        Row object(const char* name) const {
            const cJSON* value = field(name);
            return Row(checked(value, cJSON_IsObject(value) != 0, name));
        }

        const char* text(const char* name) const {
            const cJSON* value = field(name);
            return checked(value, cJSON_IsString(value) != 0, name)->valuestring;
        }

        double number(const char* name) const {
            const cJSON* value = field(name);
            if (cJSON_IsNull(value)) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            return checked(value, cJSON_IsNumber(value) != 0, name)->valuedouble;
        }

        int integer(const char* name) const { return static_cast<int>(number(name)); }

        const cJSON* json() const { return node; }

    private:
        const cJSON* field(const char* name) const { return cJSON_GetObjectItemCaseSensitive(node, name); }

        const cJSON* node;
    };

    class Table {
    public:
        explicit Table(const char* section)
            : node(array(root(), section)) {}

        Table(const char* section, const char* mode)
            : node(array(Row(root()).object(section).json(), mode)) {}

        int size() const { return cJSON_GetArraySize(node); }

        Row operator[](int index) const { return Row(cJSON_GetArrayItem(node, index)); }

    private:
        const cJSON* node;
    };
} // namespace parity

#endif
