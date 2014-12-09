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

#include "datavector.h"
#include "mask.h"

namespace
{
struct DataVectorFixture
{
    DataVectorFixture() :
        vector({1, 2, 3, 4, 5}),
        mask(5)
    {
    }

    DataVector vector;
    Mask mask;
};
}

BOOST_FIXTURE_TEST_SUITE(DataVector_tests, DataVectorFixture)

BOOST_AUTO_TEST_CASE(CalcMean_Return3)
{
    BOOST_CHECK_CLOSE(vector.CalcMean(mask), 3., 1e-10);
}

BOOST_AUTO_TEST_CASE(CalcMean_DisableLast2PointsWithMask_Return2)
{
    std::vector<bool>& m = mask.BeginEditMask();
    m[3] = false;
    m[4] = false;
    mask.EndEditMask();
    BOOST_CHECK_CLOSE(vector.CalcMean(mask), 2., 1e-10);
}

BOOST_AUTO_TEST_CASE(CalcMean_AllPointsDisabled_ReturnNAN)
{
    mask.InvertMask();
    BOOST_CHECK(std::isnan(vector.CalcMean(mask)));
}

BOOST_AUTO_TEST_CASE(CalcStdDev_Return1_5811)
{
    BOOST_CHECK_CLOSE(vector.CalcStdDev(mask), 1.5811388301, 1e-9);
}

BOOST_AUTO_TEST_CASE(CalcStdDev_DisableLast2PointsWithMask_Return1)
{
    std::vector<bool>& m = mask.BeginEditMask();
    m[3] = false;
    m[4] = false;
    mask.EndEditMask();
    BOOST_CHECK_CLOSE(vector.CalcStdDev(mask), 1., 1e-9);
}

BOOST_AUTO_TEST_CASE(CalcStdDev_AllPointsDisabled_ReturnNAN)
{
    mask.InvertMask();
    BOOST_CHECK(std::isnan(vector.CalcStdDev(mask)));
}

BOOST_AUTO_TEST_SUITE_END()

namespace
{
struct TwoDataVectorsFixture : public DataVectorFixture
{
    TwoDataVectorsFixture() :
        DataVectorFixture(),
        vector2({6,8,8,9,0})
    {
    }

    DataVector vector2;
};
}

BOOST_FIXTURE_TEST_SUITE(DataVector_Correlation_tests, TwoDataVectorsFixture)

BOOST_AUTO_TEST_CASE(CalcCorrelation_ReturnM0_47871)
{
    BOOST_CHECK_CLOSE(vector.CalcCorrelation(vector2, mask),
                      -0.4787135539,
                      1e-8);
}

BOOST_AUTO_TEST_CASE(CalcCorrelation_DisableLast2PointsWithMask_Return0_8660)
{
    std::vector<bool>& m = mask.BeginEditMask();
    m[3] = false;
    m[4] = false;
    mask.EndEditMask();
    BOOST_CHECK_CLOSE(vector.CalcCorrelation(vector2, mask),
                      0.8660254038,
                      1e-8);
}

BOOST_AUTO_TEST_CASE(CalcCorrelation_AllPointsDisabled_ReturnNAN)
{
    mask.InvertMask();
    BOOST_CHECK(std::isnan(vector.CalcCorrelation(vector2, mask)));
}

BOOST_AUTO_TEST_SUITE_END()