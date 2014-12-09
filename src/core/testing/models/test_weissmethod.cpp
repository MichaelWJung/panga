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
#include "core/models/weissmethod.h"

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
        WeissMethod method;
        std::shared_ptr<ParameterAccessor> accessor;
        Eigen::VectorXd vec;

        double& T;
        double& S;
        double& p;
    };
}

BOOST_FIXTURE_TEST_SUITE(WeissMethod_tests, Fixture)

BOOST_AUTO_TEST_CASE(CalculateConcentration)
{
    BOOST_CHECK_THROW(method.CalculateConcentration(accessor, Gas::XE), std::runtime_error);

    T = 0;
    S = 1;
    p = 2;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE), 9.77748738846232e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE), 4.48172212527456e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR), 9.91230382674763e-04, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR), 2.47039748481394e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3),  1.328780665567e-13, 1e-6);


    T = 10;
    S = .1;
    p = 1;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE), 4.64269706836073e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE), 2.01619787901304e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR), 3.85849249750526e-04, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR), 9.09654355930234e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3),  6.315021552700E-14, 1e-6);

    T = 20;
    S = 0;
    p = 1.01;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE), 4.52241473452391e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE), 1.86998524534805e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR), 3.15103538968795e-04, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR), 7.03793929300997e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3),  6.156710522179E-14, 1e-6);
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

    BOOST_CHECK_THROW(method.CalculateDerivatives(accessor, dcol, Gas::XE), std::runtime_error);

    T = 20;
    S = 0;
    p = 1.01;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::HE));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  4.58224843117053e-8 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.39367393359350e-10, 1e-8);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -2.35966230954645e-10, 1e-8);

    T = 23;
    S = .001;
    p = .99;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::NE));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.86125977680312e-7 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.31571642279490e-9 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -9.95245051655393e-10, 1e-8);

    T = 0;
    S = 10;
    p = 10;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::AR));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  4.63811930704642e-4 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.24682052351936e-4 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -3.57122182199191e-5 , 1e-8);

    T = 1;
    S = 2;
    p = 3;
    BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::KR));

    BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.18975732781123e-7 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.15742902312524e-8 , 1e-8);
    BOOST_CHECK_CLOSE(derivatives[1/*S*/], -2.79577411713379e-9 , 1e-8);
}

BOOST_AUTO_TEST_SUITE_END()
