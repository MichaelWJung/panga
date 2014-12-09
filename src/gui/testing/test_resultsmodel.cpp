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

#include <boost/math/distributions.hpp>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <gmock/gmock.h>

#include <QString>

#include <string>
#include <vector>

#include "core/fitting/fitresults.h"
#include "standardfitresultsmodel.h"

#include "core/testing/fitting/fitresults_mock.h"

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::Mock;

BOOST_AUTO_TEST_SUITE(StandardFitResultsModel_tests)

BOOST_AUTO_TEST_CASE(columnCount_2FitParameters5Gases_Return49)
{
    StandardFitResultsModel model(
        {},
        {"A", "B"},
        {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE});

    BOOST_CHECK_EQUAL(model.columnCount(), 49);
}

BOOST_AUTO_TEST_CASE(columnCount_5FitParameters5Gases_Return64)
{
    StandardFitResultsModel model(
        {},
        {"A", "B", "C", "D", "E"},
        {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE});

    BOOST_CHECK_EQUAL(model.columnCount(), 64);
}

BOOST_AUTO_TEST_CASE(columnCount_2FitParameters3Gases_Return47)
{
    StandardFitResultsModel model(
        {},
        {"A", "B"},
        {Gas::NE, Gas::AR, Gas::XE});

    BOOST_CHECK_EQUAL(model.columnCount(), 47);
}

BOOST_AUTO_TEST_CASE(data_InvalidModelIndex_ReturnInvalidQVariant)
{
    StandardFitResultsModel model({}, {}, {});
    BOOST_CHECK(model.data(QModelIndex()) == QVariant());
}

BOOST_AUTO_TEST_SUITE_END()

namespace
{
struct StandardFitResultsModel2Samples2ParametersFixture
{
    StandardFitResultsModel2Samples2ParametersFixture() :
        model({"X", "A"}, {"A", "B"}, {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE}),
        results(boost::make_shared<MockFitResults>()),
        sample_names({"X"}),
        parameters({"A", "B"}),
        concentrations()
    {
        results->chi_square = 0.0;
        results->best_estimate.resize(2);
        results->deviations.resize(2);
        results->n_iterations = 0;
        results->degrees_of_freedom = 0;
        results->equilibrium_concentrations.resize(1);
        results->model_concentrations.resize(1);
        for (GasType gas = Gas::begin; gas != Gas::end; ++gas)
        {
            results->equilibrium_concentrations[0][gas] = Data(0, 0);
            results->model_concentrations      [0][gas] = Data(0, 0);
        }
    }

    virtual ~StandardFitResultsModel2Samples2ParametersFixture()
    {
    }

    void ProcessResults()
    {
        model.ProcessResult(results, sample_names, parameters, concentrations);
    }

    StandardFitResultsModel model;
    boost::shared_ptr<MockFitResults> results;
    std::vector<std::string> sample_names;
    std::vector<std::string> parameters;
    std::vector<SampleConcentrations> concentrations;
};
}

BOOST_FIXTURE_TEST_SUITE(StandardFitResultsModel_data_tests,
                         StandardFitResultsModel2Samples2ParametersFixture)

BOOST_AUTO_TEST_CASE(data_RequestAtIndex00_ReturnDegreesOfFreedom)
{
    results->degrees_of_freedom = 3;
    ProcessResults();
    BOOST_CHECK_EQUAL(results->degrees_of_freedom,
                      model.data(model.index(0, 0)).toUInt());
}

BOOST_AUTO_TEST_CASE(data_RequestAt01_ReturnChiSquare)
{
    results->chi_square = 1.0;
    ProcessResults();
    BOOST_CHECK_CLOSE(results->chi_square,
                      model.data(model.index(0, 1)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt02_ReturnProbability)
{
    using boost::math::cdf;
    using boost::math::chi_squared;
    results->degrees_of_freedom = 1;
    results->chi_square = 1.0;
    ProcessResults();
    BOOST_CHECK_CLOSE(100. * (1. - cdf(chi_squared(results->degrees_of_freedom),
                                                   results->chi_square)),
                      model.data(model.index(0, 2)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt03And05_ReturnParameterBestEstimates)
{
    results->best_estimate.resize(2);
    results->best_estimate << 1.0, 2.0;
    ProcessResults();
    BOOST_CHECK_CLOSE(results->best_estimate[0],
                      model.data(model.index(0, 3)).toDouble(), 1e-10);
    BOOST_CHECK_CLOSE(results->best_estimate[1],
                      model.data(model.index(0, 5)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt04And06_ReturnParameterDeviations)
{
    results->deviations.resize(2);
    results->deviations << 0.1, 0.2;
    ProcessResults();
    BOOST_CHECK_CLOSE(results->deviations[0],
                      model.data(model.index(0, 4)).toDouble(), 1e-10);
    BOOST_CHECK_CLOSE(results->deviations[1],
                      model.data(model.index(0, 6)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt04And06WithEmptyDeviationsMatrix_ReturnNaN)
{
    results->deviations.resize(0);
    ProcessResults();
    BOOST_CHECK(std::isnan(model.data(model.index(0, 4)).toDouble()));
    BOOST_CHECK(std::isnan(model.data(model.index(0, 6)).toDouble()));
}

// BOOST_AUTO_TEST_CASE(data_RequestAt07WithExitFlagConverged_ReturnConverged)
// {
//     results->exit_flags = FitResults::CONVERGED;
//     ProcessResults();
//     BOOST_CHECK_EQUAL(std::string("fit converged"),
//                       model.data(model.index(0, 7)).toString().toStdString());
// }
//
// BOOST_AUTO_TEST_CASE(data_RequestAt07WithExitFlagHighL_ReturnHighL)
// {
//     results->exit_flags = FitResults::HIGH_A;
//     ProcessResults();
//     BOOST_CHECK_EQUAL(std::string("too high lambda"),
//                       model.data(model.index(0, 7)).toString().toStdString());
// }
//
// BOOST_AUTO_TEST_CASE(data_RequestAt07WithExitFlagIterationL_ReturnIterationL)
// {
//     results->exit_flags = FitResults::TOO_MANY_ITERATIONS;
//     ProcessResults();
//     BOOST_CHECK_EQUAL(std::string("iteration limit exceeded"),
//                       model.data(model.index(0, 7)).toString().toStdString());
// }
//
// BOOST_AUTO_TEST_CASE(data_RequestAt07WithExitFlagNothing_ReturnNothing)
// {
//     results->exit_flags = FitResults::NOTHING_TO_FIT;
//     ProcessResults();
//     BOOST_CHECK_EQUAL(std::string("nothing to fit"),
//                       model.data(model.index(0, 7)).toString().toStdString());
// }
//
// BOOST_AUTO_TEST_CASE(data_RequestAt07WithExitFlagError_ReturnError)
// {
//     results->exit_flags = FitResults::ERROR;
//     ProcessResults();
//     BOOST_CHECK_EQUAL(std::string("error"),
//                       model.data(model.index(0, 7)).toString().toStdString());
// }

BOOST_AUTO_TEST_CASE(data_RequestAt08_ReturnCovariance)
{
    EXPECT_CALL(*results, CorrelationCoefficient(0, 1))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(7.));
    ProcessResults();
    BOOST_CHECK_CLOSE(7., model.data(model.index(0, 8)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt09_ReturnHeResidual)
{
    results->residuals.resize(5);
    results->residuals(0) = 1.0;
    results->residual_gases = {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE};
    ProcessResults();
    BOOST_CHECK_CLOSE(results->residuals(0),
                      model.data(model.index(0, 9)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt012_ReturnKrResidual)
{
    results->residuals.resize(5);
    results->residuals(3) = 2.0;
    results->residual_gases = {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE};
    ProcessResults();
    BOOST_CHECK_CLOSE(results->residuals(3),
                      model.data(model.index(0, 12)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt014And015_ReturnHeEqConcentrationAndError)
{
    results->equilibrium_concentrations.resize(1);
    results->equilibrium_concentrations[0][Gas::HE] = Data(3.0, 0.3);
    ProcessResults();
    BOOST_CHECK_CLOSE(results->equilibrium_concentrations[0][Gas::HE].value,
                      model.data(model.index(0, 14)).toDouble(), 1e-10);
    BOOST_CHECK_CLOSE(results->equilibrium_concentrations[0][Gas::HE].error,
                      model.data(model.index(0, 15)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt018And019_ReturnArEqConcentrationAndError)
{
    results->equilibrium_concentrations.resize(1);
    results->equilibrium_concentrations[0][Gas::AR] = Data(4.0, 0.4);
    ProcessResults();
    BOOST_CHECK_CLOSE(results->equilibrium_concentrations[0][Gas::AR].value,
                      model.data(model.index(0, 18)).toDouble(), 1e-10);
    BOOST_CHECK_CLOSE(results->equilibrium_concentrations[0][Gas::AR].error,
                      model.data(model.index(0, 19)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_CASE(data_RequestAt026And027_ReturnNeModConcentrationAndError)
{
    results->model_concentrations.resize(1);
    results->model_concentrations[0][Gas::NE] = Data(5.0, 0.5);
    ProcessResults();
    BOOST_CHECK_CLOSE(results->model_concentrations[0][Gas::NE].value,
                      model.data(model.index(0, 28)).toDouble(), 1e-10);
    BOOST_CHECK_CLOSE(results->model_concentrations[0][Gas::NE].error,
                      model.data(model.index(0, 29)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_SUITE_END()

namespace
{
struct StandardFitResultsModel2Samples5ParametersFixture
{
    StandardFitResultsModel2Samples5ParametersFixture() :
        model({"X", "Y"}, {"A", "B", "C", "D", "E"},
              {Gas::HE, Gas::NE, Gas::AR, Gas::KR, Gas::XE}),
        results(boost::make_shared<MockFitResults>()),
        sample_names({"X"}),
        parameters({"A", "B"}),
        concentrations()
    {
        results->chi_square = 0.0;
        results->best_estimate.resize(5);
        results->n_iterations = 0;
        results->degrees_of_freedom = 0;
        results->equilibrium_concentrations.resize(1);
        results->model_concentrations.resize(1);
        for (GasType gas = Gas::begin; gas != Gas::end; ++gas)
        {
            results->equilibrium_concentrations[0][gas] = Data(0, 0);
            results->model_concentrations      [0][gas] = Data(0, 0);
        }
    }

    void ProcessResults()
    {
        model.ProcessResult(results, sample_names, parameters, concentrations);
    }

    StandardFitResultsModel model;
    boost::shared_ptr<MockFitResults> results;
    std::vector<std::string> sample_names;
    std::vector<std::string> parameters;
    std::vector<SampleConcentrations> concentrations;
};
}

BOOST_FIXTURE_TEST_SUITE(StandardFitResultsModel_data2_tests,
                         StandardFitResultsModel2Samples5ParametersFixture)

BOOST_AUTO_TEST_CASE(data_RequestAt121_ReturnCovarianceOfParameters52)
{
    EXPECT_CALL(*results, CorrelationCoefficient(1, 4))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(5.));
    ProcessResults();
    ProcessResults();
    BOOST_CHECK_CLOSE(5.,
                      model.data(model.index(1, 21)).toDouble(), 1e-10);
}

BOOST_AUTO_TEST_SUITE_END()

namespace
{
struct StandardFitResultsModelHeaderDataTestFixture :
        StandardFitResultsModel2Samples2ParametersFixture
{
    StandardFitResultsModelHeaderDataTestFixture() :
        StandardFitResultsModel2Samples2ParametersFixture()
    {
        ProcessResults();
    }
};
}

BOOST_FIXTURE_TEST_SUITE(StandardFitResultsModel_headerData_tests,
                         StandardFitResultsModelHeaderDataTestFixture)

BOOST_AUTO_TEST_CASE(headerData_RequestAtRow0_ReturnSampleName)
{
    BOOST_CHECK_EQUAL(sample_names[0],
                    model.headerData(0, Qt::Vertical).toString().toStdString());
}

BOOST_AUTO_TEST_CASE(headerData_RequestAtRow1_Return2ndSampleName)
{
    sample_names[0] = "A";
    ProcessResults();
    BOOST_CHECK_EQUAL(sample_names[0],
                      model.headerData(1, Qt::Vertical).toString().
                              toStdString());
}

BOOST_AUTO_TEST_CASE(headerData_RequestAtDifferentCols_ReturnAppropriateHeaders)
{
    BOOST_CHECK_EQUAL(std::string("DoF"),
                      model.headerData(0, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Chi Square"),
                      model.headerData(1, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Prob [%]"),
                      model.headerData(2, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string(parameters[0]),
                      model.headerData(3, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string(parameters[0]) + "_err",
                      model.headerData(4, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string(parameters[1]) + "_err",
                      model.headerData(6, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Convergence"),
                      model.headerData(7, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL("Corr_" + parameters[0] + "_" + parameters[1],
                      model.headerData(8, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Res_He"),
                      model.headerData(9, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Res_Xe"),
                      model.headerData(13, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Eq_He [ccSTP/g]"),
                      model.headerData(14, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Eq_Ar_err [ccSTP/g]"),
                      model.headerData(19, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Mod_He [ccSTP/g]"),
                      model.headerData(26, Qt::Horizontal).toString().
                              toStdString());
    BOOST_CHECK_EQUAL(std::string("Mod_Xe_err [ccSTP/g]"),
                      model.headerData(35, Qt::Horizontal).toString().
                              toStdString());
}

BOOST_AUTO_TEST_SUITE_END()