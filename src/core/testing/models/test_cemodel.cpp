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

#include <boost/math/special_functions/fpclassify.hpp>

#include <models/parametermanager.h>
#include <models/weissmethod.h>
#include <models/clevermethod.h>

#include "core/models/cemodel.h"

struct CeModelFixture
{
    std::shared_ptr<ParameterManager> manager;
    std::shared_ptr<ExcessAirModel> model;
    std::shared_ptr<CEqCalculationMethod> weiss;
    std::shared_ptr<CEqCalculationMethod> clever;

    std::shared_ptr<ParameterAccessor>   weiss_accessor;
    std::shared_ptr<DerivativeCollector> weiss_collector;
    std::shared_ptr<ParameterAccessor>   clever_accessor;
    std::shared_ptr<DerivativeCollector> clever_collector;
    std::shared_ptr<ParameterAccessor>   ce_accessor;
    std::shared_ptr<DerivativeCollector> ce_collector;

    Eigen::VectorXd parameters;
    Eigen::VectorXd derivatives;

    double& T;
    double& S;
    double& p;
    double& A;
    double& F;

    CeModelFixture() :
        manager(new ParameterManager()),
        model  (new CeModel(manager)),
        weiss  (new WeissMethod(manager)),
        clever (new CleverMethod(manager)),
        weiss_accessor  (weiss->GetParameterAccessor()),
        weiss_collector (weiss->GetDerivativeCollector()),
        clever_accessor (clever->GetParameterAccessor()),
        clever_collector(clever->GetDerivativeCollector()),
        ce_accessor     (model->GetParameterAccessor()),
        ce_collector    (model->GetDerivativeCollector()),
        parameters      (manager->GetParametersInOrder().size()),
        derivatives     (manager->GetParametersInOrder().size()),
        T(parameters[manager->GetParameterIndex("T")]),
        S(parameters[manager->GetParameterIndex("S")]),
        p(parameters[manager->GetParameterIndex("p")]),
        A(parameters[manager->GetParameterIndex("A")]),
        F(parameters[manager->GetParameterIndex("F")])
    {
        weiss_accessor ->SetVectorReference(parameters);
        clever_accessor->SetVectorReference(parameters);
        ce_accessor    ->SetVectorReference(parameters);
    }

    ~CeModelFixture()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(CeModel_tests, CeModelFixture)

BOOST_AUTO_TEST_CASE(CalculateConcentration)
{
    BOOST_CHECK(boost::math::isnan(model->CalculateConcentration(0, ce_accessor, Gas::HE)));

    T = 10;
    S = .1;
    p = 1;
    A = .01;
    F = .2;
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::HE),
                          ce_accessor,
                          Gas::HE),
                      8.06269744384628e-8,
                      1e-6);
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::NE),
                          ce_accessor,
                          Gas::NE),
                      3.24838579761590e-7,
                      1e-6);
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::AR),
                          ce_accessor,
                          Gas::AR),
                      0.000457118894551586,
                      1e-6);
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::KR),
                          ce_accessor,
                          Gas::KR),
                      9.98624370101141e-8,
                      1e-6);
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          clever->CalculateConcentration(clever_accessor, Gas::XE),
                          ce_accessor,
                          Gas::XE),
                      1.38516280692637e-8,
                      1e-6);

    A = 31337;
    F = 3.1415926535;
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::KR),
                          ce_accessor,
                          Gas::KR),
                      2.89552477116606e-8,
                      1e-6);

    A = 42;
    F = 1776;
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::KR),
                          ce_accessor,
                          Gas::KR),
                      5.13165315060e-11,
                      1e-6);

    T = 0;
    S = 1;
    p = 2;
    A = 3;
    F = 4;
    BOOST_CHECK_CLOSE(model->CalculateConcentration(
                          weiss->CalculateConcentration(weiss_accessor, Gas::KR),
                          ce_accessor,
                          Gas::KR),
                      6.5046455922903e-8,
                      1e-6);
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives)
{
    std::vector<int> indices;
    indices.push_back(manager->GetParameterIndex("T"));
    indices.push_back(manager->GetParameterIndex("S"));
    indices.push_back(manager->GetParameterIndex("p"));
    indices.push_back(manager->GetParameterIndex("A"));
    indices.push_back(manager->GetParameterIndex("F"));
    indices.push_back(1776);

    Eigen::RowVectorXd derivatives(6);
    derivatives.setZero();

    weiss_collector ->SetDerivativesAndResultsVector(derivatives, indices);
    clever_collector->SetDerivativesAndResultsVector(derivatives, indices);
    ce_collector    ->SetDerivativesAndResultsVector(derivatives, indices);

    T = 10;
    S = .1;
    p = 1;
    A = .01;
    F = .2;

    weiss->CalculateDerivatives(weiss_accessor, weiss_collector, Gas::KR);

    double dT = derivatives[0];
    double dS = derivatives[1];
    double dp = derivatives[2];

    BOOST_CHECK_CLOSE(derivatives[3], 0, 1e-14);
    BOOST_CHECK_CLOSE(derivatives[4], 0, 1e-14);
    BOOST_CHECK_CLOSE(derivatives[5], 0, 1e-14);

    model->CalculateDerivatives(
                weiss->CalculateConcentration(weiss_accessor, Gas::KR),
                ce_accessor,
                ce_collector,
                Gas::KR);

    BOOST_CHECK_CLOSE(derivatives[0],  1.00239152277541 * dT, 1e-6);
    BOOST_CHECK_CLOSE(derivatives[1],  1.00239152277541 * dS, 1e-6);
    BOOST_CHECK_CLOSE(derivatives[2],  1.00239152277541 * dp, 1e-6);
    BOOST_CHECK_CLOSE(derivatives[3],  8.6794555060949e-7   , 1e-6);
    BOOST_CHECK_CLOSE(derivatives[4], -1.2208981326297e-8   , 1e-6);
    BOOST_CHECK_CLOSE(derivatives[5], 0, 1e-14);
}

BOOST_AUTO_TEST_SUITE_END()
