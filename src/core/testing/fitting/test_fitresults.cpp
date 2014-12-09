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
#include <boost/concept_check.hpp>

#include "core/fitting/fitresults.h"

BOOST_AUTO_TEST_SUITE(FitResults_tests)

BOOST_AUTO_TEST_CASE(CorrelationCoefficient)
{
    FitResults r;
    r.deviations.resize(2);
    r.deviations[0] = 1.;
    r.deviations[1] = 2.;
    r.covariance_matrix.resize(2, 2);
    r.covariance_matrix(0, 0) = 1.;
    r.covariance_matrix(0, 1) = 2.;
    r.covariance_matrix(1, 0) = 3.;
    r.covariance_matrix(1, 1) = 4.;
    BOOST_CHECK_CLOSE(r.CorrelationCoefficient(0, 1), 1  , 1e-10);
    BOOST_CHECK_CLOSE(r.CorrelationCoefficient(1, 0), 1.5, 1e-10);
}

BOOST_AUTO_TEST_SUITE_END()