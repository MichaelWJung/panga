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

#include <Eigen/Core>
#include <stdexcept>

#include "core/models/weissmethod.h"

BOOST_AUTO_TEST_SUITE(WeissParameterAccessor_tests)

BOOST_AUTO_TEST_CASE(CreateAccessorAndAccessParameters)
{
    WeissParameterAccessor* w = new WeissParameterAccessor(4, 0, 1);
    ParameterAccessor* p = w;

    Eigen::VectorXd vec1(6);
    vec1 << 0.1, 1.1, 2.1, 3.1, 4.1, 5.1;

    BOOST_CHECK_NO_THROW(p->SetVectorReference(vec1));

    BOOST_CHECK_CLOSE(w->p(), 4.1, 1e-10);
    BOOST_CHECK_CLOSE(w->S(), 0.1, 1e-10);
    BOOST_CHECK_CLOSE(w->T(), 1.1, 1e-10);

    vec1[0] = 0.2;
    vec1[1] = 1.2;
    vec1[2] = 2.2;
    vec1[3] = 3.2;
    vec1[4] = 4.2;

    BOOST_CHECK_CLOSE(w->p(), 4.2, 1e-10);
    BOOST_CHECK_CLOSE(w->S(), 0.2, 1e-10);
    BOOST_CHECK_CLOSE(w->T(), 1.2, 1e-10);

    Eigen::VectorXd empty;

    BOOST_CHECK_THROW(p->SetVectorReference(empty), std::invalid_argument);

    Eigen::VectorXd vec2(5);
    vec2 << 0.3, 1.3, 2.3, 3.3, 4.3;

    BOOST_CHECK_NO_THROW(p->SetVectorReference(vec2));

    BOOST_CHECK_CLOSE(w->p(), 4.3, 1e-10);
    BOOST_CHECK_CLOSE(w->S(), 0.3, 1e-10);
    BOOST_CHECK_CLOSE(w->T(), 1.3, 1e-10);

    delete w;
}

BOOST_AUTO_TEST_SUITE_END()
