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

#include <memory>

#include <stdexcept>

#include "core/models/parametermanager.h"
#include "core/models/clevermethod.h"
#include "core/fitting/noblefitfunction.h"


namespace
{
    struct Fixture
    {
        Fixture() :
            manager(new ParameterManager()),
            method(manager),
            accessor(method.GetParameterAccessor()),
            vec(manager->GetParametersInOrder().size()),
            T(vec[manager->GetParameterIndex("T")]),
            S(vec[manager->GetParameterIndex("S")]),
            p(vec[manager->GetParameterIndex("p")])
        {
            accessor->SetVectorReference(vec);
        }

        ~Fixture()
        {
        }

        std::shared_ptr<ParameterManager> manager;
        CleverMethod method;
        std::shared_ptr<ParameterAccessor> accessor;
        Eigen::VectorXd vec;

        double& T;
        double& S;
        double& p;
    };
}

BOOST_FIXTURE_TEST_SUITE(CleverMethod_tests, Fixture)

BOOST_AUTO_TEST_CASE(CalculateConcentration)
{
    BOOST_CHECK_THROW(method.CalculateConcentration(accessor, Gas::HE), std::runtime_error);
    BOOST_CHECK_THROW(method.CalculateConcentration(accessor, Gas::NE), std::runtime_error);
    BOOST_CHECK_THROW(method.CalculateConcentration(accessor, Gas::AR), std::runtime_error);
    BOOST_CHECK_THROW(method.CalculateConcentration(accessor, Gas::KR), std::runtime_error);

    p = 1.01;
    S = 0;
    T = 20;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE), 9.60291390689642e-09, 1e-6);

    p = .99;
    S = .001;
    T = 23;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE), 8.60938089884059e-09, 1e-6);

    p = 4;
    S = 5;
    T = 6;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE), 5.88025826763174e-08, 1e-6);
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives)
{
    std::shared_ptr<DerivativeCollector> dcol(method.GetDerivativeCollector());
    Eigen::RowVectorXd derivatives(5);

    std::vector<int> indices;
    indices.push_back(17);
    indices.push_back(manager->GetParameterIndex("S"));
    indices.push_back(manager->GetParameterIndex("p"));
    indices.push_back(42);
    indices.push_back(manager->GetParameterIndex("T"));

    dcol->SetDerivativesAndResultsVector(derivatives, indices);

    BOOST_CHECK_THROW(method.CalculateDerivatives(accessor, dcol, Gas::HE), std::runtime_error);
    BOOST_CHECK_THROW(method.CalculateDerivatives(accessor, dcol, Gas::NE), std::runtime_error);
    BOOST_CHECK_THROW(method.CalculateDerivatives(accessor, dcol, Gas::AR), std::runtime_error);
    BOOST_CHECK_THROW(method.CalculateDerivatives(accessor, dcol, Gas::KR), std::runtime_error);

    T = 20;
    S = 0;
    p = 1.01;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::XE));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  9.74048724844832e-9 , 1);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -2.90137245940027e-10, 1);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -6.92150669107957e-11, 1);

    T = 23;
    S = .001;
    p = .99;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::XE));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  8.95647893243745e-9 , 1);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -2.49587521518215e-10, 1);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -6.06685983470322e-11, 1);

    T = 0;
    S = 10;
    p = 10;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::XE));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.77390654807617e-8 , 1);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -7.01099653242896e-9 , 1);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -1.56009965699347e-9 , 1);

    T = 1;
    S = 2;
    p = 3;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::XE));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.82818197939291e-8 , 1);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -2.17636629228063e-9 , 1);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -4.71408239489161e-10, 1);
}

BOOST_AUTO_TEST_SUITE_END()
