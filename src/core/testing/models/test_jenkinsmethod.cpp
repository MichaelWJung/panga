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
#include "core/models/jenkinsmethod.h"
#include <typeinfo>
#include <iostream>

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
        JenkinsMethod method;
        std::shared_ptr<ParameterAccessor> accessor;
        Eigen::VectorXd vec;

        double& T;
        double& S;
        double& p;
    };
}

BOOST_FIXTURE_TEST_SUITE(JenkinsMethod_tests, Fixture)

BOOST_AUTO_TEST_CASE(CalculateConcentration)
{   
    // Comparison with concentrations calculated with a Matlab skript
    // using following physical properties:
    // vpress (water vapor pressure) from Dickson2007/WagnerPruss2002/IAPWS-formulation1995
    // Vmol (second virial) from Dymond and Smith 1980

    T = 20;
    S = 0;
    p = 1;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE),  4.544385343568145e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE),  1.878948951786380e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR),  3.137094392548901e-04, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR),  7.020546527474003e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE),  9.632920511893169e-09, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3), 6.18662079972421E-14,  1e-6);

    T = 0;
    S = 1;
    p = 2;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE),  9.94671658952206e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE),  4.55243195998293e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR),  0.00100052397769738,  1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR),  2.49578519036359e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE),  3.89712229137032e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3), 1.35177926239257E-13,   1e-6);

    T = 10;
    S = .1;
    p = 1;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE),  4.71946504773741e-08,  1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE),  2.04235195092371e-07,  1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR),  3.880914456447179e-04, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR),  9.15504077615987e-08,  1e-6);   
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE),  1.331216894740430e-08, 1e-6);  
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3), 6.41944177163345E-14,  1e-6);    

    T = 20;
    S = 0;
    p = 1.01;
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE),  4.590903108209871e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::NE),  1.898182467103633e-07, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::AR),  3.169206682237981e-04, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::KR),  7.092411060591457e-08, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::XE),  9.731526130762817e-09, 1e-6);
    BOOST_CHECK_CLOSE(method.CalculateConcentration(accessor, Gas::HE3), 6.24994900552799E-14,  1e-6);
}

// BOOST_AUTO_TEST_CASE(CalculateDerivatives)
// {
//     std::shared_ptr<DerivativeCollector> dcol(method.GetDerivativeCollector());
//     Eigen::RowVectorXd derivatives(5);

//     std::vector<int> indices;
//     indices.push_back(17);
//     indices.push_back(manager->GetParameterIndex("S"));
//     indices.push_back(manager->GetParameterIndex("p"));
//     indices.push_back(42);
//     indices.push_back(manager->GetParameterIndex("T"));

//     dcol->SetDerivativesAndResultsVector(derivatives, indices);

//     T = 20;
//     S = 0;
//     p = 1.01;
//     BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::HE));

//     BOOST_CHECK_CLOSE(derivatives[2/*p*/],  4.58224843117053e-8 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.39367393359350e-10, 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[1/*S*/], -2.35966230954645e-10, 1e-8);
//     std::cout<< derivatives[2/*p*/] << typeid(derivatives[2/*p*/]).name()<<std::endl;
//     T = 23;
//     S = .001;
//     p = .99;
//     BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::NE));

//     BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.86125977680312e-7 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.31571642279490e-9 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[1/*S*/], -9.95245051655393e-10, 1e-8);

//     T = 0;
//     S = 10;
//     p = 10;
//     BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::AR));

//     BOOST_CHECK_CLOSE(derivatives[2/*p*/],  4.63811930704642e-4 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.24682052351936e-4 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[1/*S*/], -3.57122182199191e-5 , 1e-8);

//     T = 1;
//     S = 2;
//     p = 3;
//     BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::KR));

//     BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.18975732781123e-7 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.15742902312524e-8 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[1/*S*/], -2.79577411713379e-9 , 1e-8);

//     T = 1;
//     S = 2;
//     p = 3;
//     BOOST_REQUIRE_NO_THROW(method.CalculateDerivatives(accessor, dcol, Gas::XE));

//     BOOST_CHECK_CLOSE(derivatives[2/*p*/],  1.18975732781123e-7 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[4/*T*/], -1.15742902312524e-8 , 1e-8);
//     BOOST_CHECK_CLOSE(derivatives[1/*S*/], -2.79577411713379e-9 , 1e-8);
// }

BOOST_AUTO_TEST_SUITE_END()
