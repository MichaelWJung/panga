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

#include "core/models/physicalproperties.h"

BOOST_AUTO_TEST_SUITE(PhysicalProperties_tests)

BOOST_AUTO_TEST_CASE(CalcSaturationVaporPressure_Gill)
{
    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcSaturationVaporPressure_Gill(0),
                6.0281407994095e-3,
                1e-6
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcSaturationVaporPressure_Gill(10),
                1.2106738021575e-2,
                1e-6
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcSaturationVaporPressure_Gill(20),
                2.3057715561949e-2,
                1e-6
                );
}

BOOST_AUTO_TEST_CASE(CalcSaturationVaporPressureDerivative_Gill)
{
    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcSaturationVaporPressureDerivative_Gill(0),
                4.37679406606328e-04,
                1e-2
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcSaturationVaporPressureDerivative_Gill(10),
                8.10861794375839e-04,
                1e-2
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcSaturationVaporPressureDerivative_Gill(20),
                1.42896111610114e-03,
                1e-2
                );
}

BOOST_AUTO_TEST_CASE(CalcWaterDensity)
{
    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcWaterDensity(1, 0, 5),
                999.96675,
                1e-4
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcWaterDensity(1, 35, 5),
                1027.67547,
                1e-4
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcWaterDensity(987.910189983, 35, 25),
                1062.53817,
                1e-4
                );
}

BOOST_AUTO_TEST_CASE(CalcMoleXeFractionSolubility)
{
    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcXeMoleFractionSolubility(0),
                0.0001799440120,
                1e-4
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcXeMoleFractionSolubility(10),
                0.0001239533243,
                1e-4
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcXeMoleFractionSolubility(20),
                0.00009053089539,
                1e-4
                );
}

BOOST_AUTO_TEST_CASE(CalcXeSaltingCoefficient)
{
    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcXeSaltingCoefficient(0),
                0.459912251,
                1e-4
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcXeSaltingCoefficient(10),
                0.412657572,
                1e-4
                );

    BOOST_CHECK_CLOSE(
                PhysicalProperties::CalcXeSaltingCoefficient(20),
                0.376522755,
                1e-4
                );
}

BOOST_AUTO_TEST_CASE(ConvertToMole)
{
    BOOST_CHECK_CLOSE(
                PhysicalProperties::ConvertToMole(1., Gas::HE),
                1./22426.42559,
                1e-4
                );
    BOOST_CHECK_CLOSE(
                PhysicalProperties::ConvertToMole(1., Gas::NE),
                1./22424.21021,
                1e-4
                );
    BOOST_CHECK_CLOSE(
                PhysicalProperties::ConvertToMole(1., Gas::AR),
                1./22392.52611,
                1e-4
                );
    BOOST_CHECK_CLOSE(
                PhysicalProperties::ConvertToMole(1., Gas::KR),
                1./22352.05331,
                1e-4
                );
    BOOST_CHECK_CLOSE(
                PhysicalProperties::ConvertToMole(1., Gas::XE),
                1./22259.39172,
                1e-4
                );
}

BOOST_AUTO_TEST_CASE(ConvertToMolePerLiter)
{
    BOOST_CHECK_CLOSE(
            PhysicalProperties::ConvertToMolePerLiter(1., 1, 0, 5, Gas::HE),
            1./22426.42559 * PhysicalProperties::CalcWaterDensity(1, 0, 5),
            1e-4
            );
    BOOST_CHECK_CLOSE(
            PhysicalProperties::ConvertToMolePerLiter(1., 1, 35, 5, Gas::NE),
            1./22424.21021 * PhysicalProperties::CalcWaterDensity(1, 35, 5),
            1e-4
            );
    BOOST_CHECK_CLOSE(
            PhysicalProperties::ConvertToMolePerLiter(1., 987.910189983, 35, 25, Gas::AR),
            1./22392.52611 * PhysicalProperties::CalcWaterDensity(987.910189983, 35, 25),
            1e-4
            );
    BOOST_CHECK_CLOSE(
            PhysicalProperties::ConvertToMolePerLiter(1., 1, 0, 5, Gas::KR),
            1./22352.05331 * PhysicalProperties::CalcWaterDensity(1, 0, 5),
            1e-4
            );
    BOOST_CHECK_CLOSE(
            PhysicalProperties::ConvertToMolePerLiter(1., 1, 35, 5, Gas::XE),
            1./22259.39172 * PhysicalProperties::CalcWaterDensity(1, 35, 5),
            1e-4
            );
}

BOOST_AUTO_TEST_CASE(DiffusionCoefficientsInWater)
{
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(0., Gas::HE),
            2.0273623729, 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(0., Gas::NE),
            1., 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(0., Gas::AR),
            0.617644178, 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(0., Gas::KR),
            0.3753380368, 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(0., Gas::XE),
            0.2842253913, 1e-8);
    
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(100., Gas::HE),
            1.3996316529, 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(100., Gas::NE),
            1., 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(100., Gas::AR),
            0.7674216219, 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(100., Gas::KR),
            0.7064964618, 1e-8);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::GetDiffusionCoefficientInWater(100., Gas::XE),
            0.6318434315, 1e-8);
}

BOOST_AUTO_TEST_CASE(Req)
{
    BOOST_CHECK_CLOSE(
            PhysicalProperties::CalcReq( 0., 1.0), 1.35902059E-06, 1e-6);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::CalcReq(10., 0.1), 1.36020538E-06, 1e-6);
    BOOST_CHECK_CLOSE(
            PhysicalProperties::CalcReq(20., 0.0), 1.36137681E-06, 1e-6);

}

BOOST_AUTO_TEST_SUITE_END()
