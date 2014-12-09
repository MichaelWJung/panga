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


//! \todo Prüfen ob dieser Code gelöscht werden kann.
/*
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <Eigen/Core>

#include <cmath>

#include "calculations/ceqcalculationmethod.h"


namespace
{

    class CEqCalculationTestMethod : public CEqCalculationMethod
    {
    public:
        ~CEqCalculationTestMethod() {}

        virtual double CalculateConcentration(const Eigen::VectorXd &parameters, GasType gas) const = 0;
        virtual void CalculateReferenceDerivative(Eigen::VectorXd& results,
                                                  const Eigen::VectorXd &parameters) const = 0;
    };

    class TrigonometryTest : public CEqCalculationTestMethod
    {

    public:

        ~TrigonometryTest() {}

        double CalculateConcentration(const Eigen::VectorXd &parameters, GasType gas) const
        {
            return std::sin(parameters[0]) + std::cos(parameters[1]) + std::exp(parameters[2]);
        }

        void CalculateReferenceDerivative(Eigen::VectorXd &results, const Eigen::VectorXd &params) const
        {
            results.resize(3);
            results[0] =  std::cos(params[0]);
            results[1] = -std::sin(params[1]);
            results[2] =  std::exp(params[2]);
        }

    };

    class FractionTest : public CEqCalculationTestMethod
    {
    public:
        ~FractionTest() {}

        double CalculateConcentration(const Eigen::VectorXd &parameters, GasType gas) const
        {
            return (2*std::pow(parameters[0], 2) - 3*std::pow(parameters[1],-3)) /
                    std::pow(parameters[2], 3);
        }

        void CalculateReferenceDerivative(Eigen::VectorXd &results, const Eigen::VectorXd &parameters) const
        {
            results.resize(3);
            results[0] = 4 * parameters[0] / std::pow(parameters[2], 3);
            results[1] = 9 / (std::pow(parameters[1], 4) * std::pow(parameters[2], 3));
            results[2] = (9*std::pow(parameters[1], -3) - 6*std::pow(parameters[0], 2))
                    / std::pow(parameters[2], 4);
        }
    };

    struct CEqCalculationMethodTestData
    {
        CEqCalculationMethodTestData(double a, double b, double c, CEqCalculationTestMethod* method) :
            method(method),
            params(3),
            derivatives(3)
        {
            params[0] = a;
            params[1] = b;
            params[2] = c;
            derivatives[0] = 0;
            derivatives[1] = 1;
            derivatives[2] = 2;
        }

        ~CEqCalculationMethodTestData()
        {
            delete method;
        }

        void CompareResults()
        {
            Eigen::RowVectorXd results(3);
            Eigen::VectorXd references(3);
            method->CalculateDerivatives(results.row(0), params, derivatives, Gas::INVALID);
            method->CalculateReferenceDerivative(references, params);

            double r = std::sqrt(std::pow(results[0], 2) +
                                 std::pow(results[1], 2) +
                                 std::pow(results[2], 2));
            double reference_r = std::sqrt(std::pow(references[0], 2) +
                                           std::pow(references[1], 2) +
                                           std::pow(references[2], 2));

            BOOST_CHECK_CLOSE(r, reference_r, .1);

            double phi = std::atan2(results[1], results[0]);
            double reference_phi = std::atan2(references[1], references[0]);

            BOOST_CHECK_LE(phi - reference_phi, 2e-2);

            double theta = std::acos(results[2] / r);
            double reference_theta = std::acos(references[2] / reference_r);

            BOOST_CHECK_LE(theta - reference_theta, 2e-2);
        }

        CEqCalculationTestMethod* method;
        Eigen::VectorXd params;
        std::vector<int> derivatives;
    };

}

BOOST_AUTO_TEST_SUITE(CEqCalculationMethod_tests)

BOOST_AUTO_TEST_CASE(CalculateDerivatives1)
{
    CEqCalculationMethodTestData test(1, 2, 3, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives2)
{
    CEqCalculationMethodTestData test(0, 1, 1, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives3)
{
    CEqCalculationMethodTestData test(0, 0, 0, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives4)
{
    CEqCalculationMethodTestData test(1000000, 1, 0, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives5)
{
    CEqCalculationMethodTestData test(10, -2, -17, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives6)
{
    CEqCalculationMethodTestData test(0, 0, 100, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives7)
{
    CEqCalculationMethodTestData test(1e-5, 1e-5, 1e-5, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives8)
{
    CEqCalculationMethodTestData test(1e-50, 1e-50, 1e-50, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives9)
{
    CEqCalculationMethodTestData test(-1, -1, -1, new TrigonometryTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives10)
{
    CEqCalculationMethodTestData test(1, 2, 3, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives11)
{
    CEqCalculationMethodTestData test(0, 1, 1, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives12)
{
    CEqCalculationMethodTestData test(0, 1e-5, 1e-5, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives13)
{
    CEqCalculationMethodTestData test(1000000, 1, 1e-5, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives14)
{
    CEqCalculationMethodTestData test(10, -2, -17, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives15)
{
    CEqCalculationMethodTestData test(0, 1e-5, 100, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives16)
{
    CEqCalculationMethodTestData test(1e-5, 1e-5, 1e-5, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_CASE(CalculateDerivatives17)
{
    CEqCalculationMethodTestData test(-1, -1, -1, new FractionTest());
    test.CompareResults();
}

BOOST_AUTO_TEST_SUITE_END()
*/
