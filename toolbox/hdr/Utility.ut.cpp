// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Utility.hpp"

#include "Histogram.hpp"
#include "Iterator.hpp"

#include <boost/test/unit_test.hpp>

#include <cmath>

using namespace std;
using namespace toolbox;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(UtilitySuite)

BOOST_AUTO_TEST_CASE(CreateWithLargeValuesCase)
{
    HdrHistogram h{20000000, 100000000, 5};
    BOOST_TEST(h.record_value(100000000));
    BOOST_TEST(h.record_value(20000000));
    BOOST_TEST(h.record_value(30000000));

    BOOST_TEST(h.values_are_equivalent(value_at_percentile(h, 50.0), 20000000));
    BOOST_TEST(h.values_are_equivalent(value_at_percentile(h, 83.33), 30000000));
    BOOST_TEST(h.values_are_equivalent(value_at_percentile(h, 83.34), 100000000));
    BOOST_TEST(h.values_are_equivalent(value_at_percentile(h, 99.0), 100000000));
}

BOOST_AUTO_TEST_CASE(HighSignificantFiguresCase)
{
    const std::initializer_list<int64_t> vals{459876,  669187,  711612,  816326,  931423,
                                              1033197, 1131895, 2477317, 3964974, 12718782};
    HdrHistogram h{459876, 12718782, 5};
    for (const auto val : vals) {
        BOOST_TEST(h.record_value(val));
    }
    BOOST_TEST(value_at_percentile(h, 50) == 1048575);
}

BOOST_AUTO_TEST_CASE(NaNCase)
{
    HdrHistogram h{1, 100000, 3};
    BOOST_TEST(isnan(mean(h)));
    BOOST_TEST(isnan(stddev(h)));
}

BOOST_AUTO_TEST_CASE(StatsCase)
{
    HdrHistogram h{1, 10000000, 3};
    for (int i{0}; i < 1000000; ++i) {
        BOOST_TEST(h.record_value(i));
    }

    BOOST_TEST(h.min() == 0);
    BOOST_TEST(h.max() == 1000447);

    BOOST_TEST(value_at_percentile(h, 50) == 500223);
    BOOST_TEST(value_at_percentile(h, 75) == 750079);
    BOOST_TEST(value_at_percentile(h, 90) == 900095);
    BOOST_TEST(value_at_percentile(h, 95) == 950271);
    BOOST_TEST(value_at_percentile(h, 99) == 990207);
    BOOST_TEST(value_at_percentile(h, 99.9) == 999423);
    BOOST_TEST(value_at_percentile(h, 99.99) == 999935);

    BOOST_TEST(mean(h) == 500000.013312);
    BOOST_TEST(stddev(h) == 288675.1403682715);
}

BOOST_AUTO_TEST_SUITE_END()
