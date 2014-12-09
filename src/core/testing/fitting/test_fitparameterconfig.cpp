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

#include <string>
#include <vector>

#include "core/fitting/fitparameterconfig.h"

namespace {
struct Fixture
{
    FitParameterConfig config;
};
}

BOOST_FIXTURE_TEST_SUITE(ParameterConfig_tests, Fixture)

BOOST_AUTO_TEST_CASE(AddParameter)
{
    config.AddParameter(FitParameter("ABC", 1));
    config.AddParameter(FitParameter("XYZ", 2));
    
    BOOST_REQUIRE_EQUAL(config.size(), 2);
    
    BOOST_CHECK_EQUAL(config.names()[0], "ABC");
    BOOST_CHECK_EQUAL(config.names()[1], "XYZ");
    
    BOOST_CHECK_CLOSE(config.initials()[0], 1, 1e-10);
    BOOST_CHECK_CLOSE(config.initials()[1], 2, 1e-10);
    
    BOOST_CHECK_THROW(config.AddParameter(FitParameter("ABC", 1)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(RemoveParameter)
{
    config.AddParameter(FitParameter("ABC", 1));
    config.AddParameter(FitParameter("FGH", 5));
    config.AddParameter(FitParameter("XYZ", 2));
    
    config.RemoveParameter(1);
    
    BOOST_REQUIRE_EQUAL(config.size(), 2);
    BOOST_REQUIRE_EQUAL(config.names().size(), 2);
    BOOST_REQUIRE_EQUAL(config.initials().size(), 2);
    
    BOOST_CHECK_EQUAL(config.names()[0], "ABC");
    BOOST_CHECK_EQUAL(config.names()[1], "XYZ");
    
    BOOST_CHECK_CLOSE(config.initials()[0], 1, 1e-10);
    BOOST_CHECK_CLOSE(config.initials()[1], 2, 1e-10);
    
    config.RemoveParameter(0);
    config.RemoveParameter(0);
    
    BOOST_CHECK_EQUAL(config.size(), 0);
    BOOST_CHECK_EQUAL(config.names().size(), 0);
    BOOST_CHECK_EQUAL(config.initials().size(), 0);
}

BOOST_AUTO_TEST_CASE(ChangeParameterInitial)
{
    config.AddParameter(FitParameter("ABC", 1));
    config.AddParameter(FitParameter("FGH", 5));
    config.AddParameter(FitParameter("XYZ", 2));
    
    config.ChangeParameterInitial(0, 7.);
    config.ChangeParameterInitial(2, 6.);
    
    BOOST_CHECK_CLOSE(config.initials()[0], 7, 1e-10);
    BOOST_CHECK_CLOSE(config.initials()[1], 5, 1e-10);
    BOOST_CHECK_CLOSE(config.initials()[2], 6, 1e-10);
}

BOOST_AUTO_TEST_CASE(ChangeParameterInitialByName)
{
    config.AddParameter(FitParameter("ABC", 1));
    config.AddParameter(FitParameter("FGH", 5));
    config.AddParameter(FitParameter("XYZ", 2));   
    
    config.ChangeParameterInitial("ABC", 7.);
    config.ChangeParameterInitial("XYZ", 6.);
    
    BOOST_CHECK_CLOSE(config.initials()[0], 7, 1e-10);
    BOOST_CHECK_CLOSE(config.initials()[1], 5, 1e-10);
    BOOST_CHECK_CLOSE(config.initials()[2], 6, 1e-10);
    
    BOOST_CHECK_THROW(config.ChangeParameterInitial("BUH", 42.),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
