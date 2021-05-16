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

#include <memory>

#include <stdexcept>

#include "core/models/clevermethod.h"
#include "core/models/weissmethod.h"
#include "core/models/weissmethodfactory.h"
#include "core/models/combinedmodel.h"

#include "testmodel.h"
#include "testceqmethod.h"

BOOST_AUTO_TEST_SUITE(CombinedModel_tests)

BOOST_AUTO_TEST_CASE(ParameterNotFound)
{   
    ModelFactory* factory(new TestFactory());
    CEqMethodFactory* ceqmethod_factory(new TestCEqFactory());
    CombinedModel model(factory, ceqmethod_factory);

    BOOST_CHECK_NO_THROW(model.GetParameterIndex("a"));
    BOOST_CHECK_NO_THROW(model.GetParameterIndex("b"));
    BOOST_CHECK_NO_THROW(model.GetParameterIndex("c"));
    BOOST_CHECK_NO_THROW(model.GetParameterIndex("T"));
    BOOST_CHECK_NO_THROW(model.GetParameterIndex("S"));
    BOOST_CHECK_NO_THROW(model.GetParameterIndex("p"));

    BOOST_CHECK_THROW(model.GetParameterIndex("x"), CombinedModel::ParameterNotFound);
    BOOST_CHECK_THROW(model.GetParameterIndex("t"), CombinedModel::ParameterNotFound);
    BOOST_CHECK_THROW(model.GetParameterIndex("C"), CombinedModel::ParameterNotFound);
}

BOOST_AUTO_TEST_CASE(ConcentrationsAndDerivatives)
{
    ModelFactory* factory(new TestFactory());
    CEqMethodFactory* ceqmethod_factory(new WeissMethodFactory()); // Use Weiss to test concentrations and derivatives
    CombinedModel model(factory, ceqmethod_factory);

    std::vector<int> indices;
    indices.push_back(model.GetParameterIndex("a"));
    indices.push_back(model.GetParameterIndex("b"));
    indices.push_back(model.GetParameterIndex("c"));
    indices.push_back(model.GetParameterIndex("T"));
    indices.push_back(model.GetParameterIndex("S"));
    indices.push_back(model.GetParameterIndex("p"));

    model.SetupDerivatives(indices);

    Eigen::VectorXd parameters(6);
    double& a = parameters[model.GetParameterIndex("a")];
    double& b = parameters[model.GetParameterIndex("b")];
    double& c = parameters[model.GetParameterIndex("c")];
    double& T = parameters[model.GetParameterIndex("T")];
    double& S = parameters[model.GetParameterIndex("S")];
    double& p = parameters[model.GetParameterIndex("p")];

    a = 2;
    b = 7;
    c = 21;
    T = 20;
    S = 0;
    p = 1.01;

    model.SetParameters(parameters);

    BOOST_CHECK_CLOSE(
                model.CalculateEquilibriumConcentration(Gas::NE),
                1.86998579479645e-07,
                1e-6
                );

    BOOST_CHECK_CLOSE(
                model.CalculateConcentration(Gas::NE),
                (a + b + c + T) * 1.86998579479645e-07,
                1e-6
                );

    Eigen::RowVectorXd derivs = model.CalculateEquilibriumDerivatives(Gas::HE);

    BOOST_REQUIRE_EQUAL(derivs.size(), 6);
    BOOST_CHECK_CLOSE(derivs[0]/*a*/, 0, 1e-14);
    BOOST_CHECK_CLOSE(derivs[1]/*b*/, 0, 1e-14);
    BOOST_CHECK_CLOSE(derivs[2]/*c*/, 0, 1e-14);
    BOOST_CHECK_CLOSE(derivs[3]/*T*/, -1.39366806606821e-10, 1e-8);
    BOOST_CHECK_CLOSE(derivs[4]/*S*/, -2.35971963482711e-10, 1e-8);
    BOOST_CHECK_CLOSE(derivs[5]/*p*/,  4.58238131136642e-08 , 1e-8);

    derivs = model.CalculateDerivatives(Gas::HE);

    BOOST_REQUIRE_EQUAL(derivs.size(), 6);
    BOOST_CHECK_CLOSE(derivs[0]/*a*/, 1, 1e-14);
    BOOST_CHECK_CLOSE(derivs[1]/*b*/, 2, 1e-14);
    BOOST_CHECK_CLOSE(derivs[2]/*c*/, 3, 1e-14);
    BOOST_CHECK_CLOSE(derivs[3]/*T*/,   42 * -1.39366806606821e-10, 1e-8);
    BOOST_CHECK_CLOSE(derivs[4]/*S*/, 1e30 * -2.35971963482711e-10, 1e-8);
    BOOST_CHECK_CLOSE(derivs[5]/*p*/, 1e30 *  4.58238131136642e-08 , 1e-8);

    BOOST_CHECK_CLOSE(
                model.CalculateEquilibriumConcentration(Gas::XE),
                9.60291390689642e-09,
                1e-6
                );

    BOOST_CHECK_CLOSE(
                model.CalculateConcentration(Gas::XE),
                (a + b + c + T) * 9.60291390689642e-09,
                1e-6
                );

    derivs = model.CalculateEquilibriumDerivatives(Gas::XE);

    BOOST_REQUIRE_EQUAL(derivs.size(), 6);
    BOOST_CHECK_CLOSE(derivs[0]/*a*/, 0, 1e-14);
    BOOST_CHECK_CLOSE(derivs[1]/*b*/, 0, 1e-14);
    BOOST_CHECK_CLOSE(derivs[2]/*c*/, 0, 1e-14);
    BOOST_CHECK_CLOSE(derivs[3]/*T*/, -2.89836210587424e-10, 1e-2);
    BOOST_CHECK_CLOSE(derivs[4]/*S*/, -6.90227958127318e-11, 1e-2);
    BOOST_CHECK_CLOSE(derivs[5]/*p*/,  9.73024428135358e-09 , 1e-2);

    derivs = model.CalculateDerivatives(Gas::XE);

    BOOST_REQUIRE_EQUAL(derivs.size(), 6);
    BOOST_CHECK_CLOSE(derivs[0]/*a*/, 1, 1e-14);
    BOOST_CHECK_CLOSE(derivs[1]/*b*/, 2, 1e-14);
    BOOST_CHECK_CLOSE(derivs[2]/*c*/, 3, 1e-14);
    BOOST_CHECK_CLOSE(derivs[3]/*T*/,   42 * -2.89836210587424e-10, 1e-2);
    BOOST_CHECK_CLOSE(derivs[4]/*S*/, 1e30 * -6.90227958127318e-11, 1e-2);
    BOOST_CHECK_CLOSE(derivs[5]/*p*/, 1e30 *  9.73024428135358e-09 , 1e-2);
}

BOOST_AUTO_TEST_CASE(SanityChecks)
{
    ModelFactory* factory(new TestFactory());
    CEqMethodFactory* ceqmethod_factory(new TestCEqFactory());
    CombinedModel model(factory, ceqmethod_factory);

    Eigen::VectorXd parameters(3);
    BOOST_CHECK_THROW(model.SetParameters(parameters), std::invalid_argument);

    parameters.resize(8);
    BOOST_CHECK_THROW(model.SetParameters(parameters), std::invalid_argument);

    parameters.resize(6);
    BOOST_CHECK_NO_THROW(model.SetParameters(parameters));

    BOOST_CHECK_EQUAL(model.CalculateDerivatives(Gas::HE).size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
