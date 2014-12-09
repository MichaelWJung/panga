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

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/models/parametermanager.h"

BOOST_AUTO_TEST_SUITE(ParameterManager_tests)

BOOST_AUTO_TEST_CASE(RegisterAndGetParameters)
{
    ParameterManager p;

    BOOST_CHECK_EQUAL(p.RegisterParameter("A", "ccSTP/g"), 0U);
    BOOST_CHECK_THROW(p.RegisterParameter("A", "1"), std::runtime_error);
    BOOST_CHECK_EQUAL(p.RegisterParameter("A", "ccSTP/g"), 0U);
    BOOST_CHECK_EQUAL(p.RegisterParameter("T", "°C"), 1U);
    BOOST_CHECK_EQUAL(p.RegisterParameter("F", "1"), 2U);
    BOOST_CHECK_EQUAL(p.RegisterParameter("T", "°C"), 1U);

    std::vector<ModelParameter> registered_parameters =
            p.GetParametersInOrder();
    std::sort(registered_parameters.begin(), registered_parameters.end());

    std::vector<ModelParameter> parameters_for_comparison =
            {ModelParameter("A", "ccSTP/g", 0., 0U),
             ModelParameter("T", "°C", 0., 1U),
             ModelParameter("F", "1", 0., 2U)};

    BOOST_REQUIRE_EQUAL(registered_parameters.size(), parameters_for_comparison.size());
    BOOST_CHECK(std::equal(registered_parameters.begin(), registered_parameters.end(),
                           parameters_for_comparison.begin()));
}

BOOST_AUTO_TEST_CASE(RegisterParametersWithBoundaries)
{
    ParameterManager p;
    
    BOOST_CHECK_NO_THROW(p.RegisterParameter("A", "B", 0, -10, 10, 5));
    BOOST_CHECK_NO_THROW(p.RegisterParameter("A", "B", 0, -15, 15, 10));
    
    std::vector<ModelParameter> registered_parameters =
            p.GetParametersInOrder();

    std::vector<ModelParameter> parameters_for_comparison =
            {ModelParameter("A", "B", 0., -10., 10., 5., 0U)};

    BOOST_REQUIRE_EQUAL(registered_parameters.size(), parameters_for_comparison.size());
    BOOST_CHECK(std::equal(registered_parameters.begin(), registered_parameters.end(),
                           parameters_for_comparison.begin()));
    
    BOOST_CHECK_NO_THROW(p.RegisterParameter("A", "B", 0, -5, 5, 2));
    
    parameters_for_comparison =
            {ModelParameter("A", "B", 0., -5., 5., 2., 0U)};
            
    BOOST_REQUIRE_EQUAL(registered_parameters.size(), parameters_for_comparison.size());
    BOOST_CHECK(std::equal(registered_parameters.begin(), registered_parameters.end(),
                           parameters_for_comparison.begin()));
}

BOOST_AUTO_TEST_CASE(GetParameterIndex)
{
    ParameterManager p;

    unsigned a = p.RegisterParameter("A", "ccSTP/g");
    BOOST_CHECK_EQUAL(a, p.GetParameterIndex("A"));

    a = p.RegisterParameter("A", "ccSTP/g");
    BOOST_CHECK_EQUAL(a, p.GetParameterIndex("A"));

    unsigned v = p.RegisterParameter("V", "Boenk");
    BOOST_CHECK_EQUAL(v, p.GetParameterIndex("V"));

    unsigned x = p.RegisterParameter("xoop", "Honk");
    BOOST_CHECK_EQUAL(x, p.GetParameterIndex("xoop"));

    BOOST_CHECK_EQUAL(a, p.GetParameterIndex("A"));
    BOOST_CHECK_EQUAL(v, p.GetParameterIndex("V"));
    BOOST_CHECK_EQUAL(x, p.GetParameterIndex("xoop"));
}

BOOST_AUTO_TEST_CASE(GetParametersInOrder)
{
    ParameterManager p;

    p.RegisterParameter("a", "1");
    p.RegisterParameter("b", "1");
    p.RegisterParameter("c", "1");
    p.RegisterParameter("a", "1");
    p.RegisterParameter("d", "1");
    p.RegisterParameter("c", "1");
    p.RegisterParameter("b", "1");

    std::vector<ModelParameter> params = p.GetParametersInOrder();

    BOOST_REQUIRE_EQUAL(params.size(), 4);

    unsigned j = 0;
    for (auto it = params.cbegin(); it != params.cend(); ++it, ++j)
        BOOST_CHECK_EQUAL(p.GetParameterIndex(it->name), j);
}

BOOST_AUTO_TEST_SUITE_END()
