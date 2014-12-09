// Copyright © 2014 Michael Jung
// 
// This file is part of Panga.
// 
// Panga is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Panga is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Panga.  If not, see <http://www.gnu.org/licenses/>.


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <stdexcept>

#include "core/misc/rundata.h"

namespace {
struct Fixture
{
    Fixture()
    {
        rundata.Add(std::make_pair("A", SampleConcentrations({{Gas::HE, Data(0, 0)}})));
        rundata.Add(std::make_pair("B", SampleConcentrations({{Gas::HE, Data(1, 1)}})));
        rundata.Add(std::make_pair("C", SampleConcentrations({{Gas::HE, Data(2, 2)}})));
        rundata.Add(std::make_pair("D", SampleConcentrations({{Gas::HE, Data(3, 3)}})));
    }
    RunData rundata;
};
}

BOOST_FIXTURE_TEST_SUITE(RunData_tests, Fixture)

BOOST_AUTO_TEST_CASE(ConcentrationsIterators_DisableSomeSamples1_IterateOnlyOverEnabledOnes)
{
    rundata.DisableSample(0);
    rundata.DisableSample(2);
    auto it = rundata.concentrations_begin();
    BOOST_CHECK_CLOSE((*  it).at(Gas::HE).value, 1, 1e-10);
    BOOST_CHECK_CLOSE((*++it).at(Gas::HE).value, 3, 1e-10);
    BOOST_CHECK(++it == rundata.concentrations_end());
}

BOOST_AUTO_TEST_CASE(ConcentrationsIterators_DisableSomeSamples2_IterateOnlyOverEnabledOnes)
{
    rundata.DisableSample(1);
    rundata.DisableSample(3);
    auto it = rundata.concentrations_begin();
    BOOST_CHECK_CLOSE((*  it).at(Gas::HE).value, 0, 1e-10);
    BOOST_CHECK_CLOSE((*++it).at(Gas::HE).value, 2, 1e-10);
    BOOST_CHECK(++it == rundata.concentrations_end());
}

BOOST_AUTO_TEST_CASE(ConcentrationsIterators_DisableSomeSamplesAndReenableOne_IterateOnlyOverEnabledOnes)
{
    rundata.DisableSample(1);
    rundata.EnableSample(1);
    rundata.DisableSample(3);
    auto it = rundata.concentrations_begin();
    BOOST_CHECK_CLOSE((*  it).at(Gas::HE).value, 0, 1e-10);
    BOOST_CHECK_CLOSE((*++it).at(Gas::HE).value, 1, 1e-10);
    BOOST_CHECK_CLOSE((*++it).at(Gas::HE).value, 2, 1e-10);
    BOOST_CHECK(++it == rundata.concentrations_end());
}

BOOST_AUTO_TEST_SUITE_END()
