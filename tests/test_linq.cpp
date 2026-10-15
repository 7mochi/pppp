// Copyright (c) ppy Pty Ltd <contact@ppy.sh>. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include "pppp/utils/linq/csharp.h"
#include <doctest.h>
#include <vector>

namespace linq = ::pppp::utils::linq;

TEST_CASE("to_list is repeatable and does not outlive its own storage") {
    std::vector<double> source;
    source.push_back(1.0);
    source.push_back(2.0);
    source.push_back(3.0);

    linq::Enumerable<double> sequence = linq::from(source).where(linq::_1 > 0.0);

    {
        const std::vector<double> first = sequence.to_list();
        CHECK(first.size() == 3u);
    }

    const std::vector<double> second = sequence.to_list();
    CHECK(second.size() == 3u);
    CHECK(second[0] == 1.0);
    CHECK(second[2] == 3.0);
}

TEST_CASE("a copied owned sequence is independent of its source") {
    std::vector<double> source;
    source.push_back(2.0);
    source.push_back(1.0);
    source.push_back(3.0);

    linq::Enumerable<double> original = linq::from(source).where(linq::_1 > 0.0);
    linq::Enumerable<double> copy = original;

    CHECK(copy.to_list() == original.to_list());

    // Consuming the copy must leave the original untouched.
    copy.take(1).to_list();
    CHECK(original.to_list().size() == 3u);
}
