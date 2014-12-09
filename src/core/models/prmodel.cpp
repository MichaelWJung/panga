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


#include <cassert>
#include <cmath>

#include <boost/assign.hpp>
#include <boost/foreach.hpp>

#include "physicalproperties.h"

#include "prmodel.h"

const std::string PrModel::NAME = "PR";

PrModel::PrModel(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(
            manager,
            "A"   , "ccSTP/g" , 0.01, -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "F_PR", "1"       , 1.  , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "T"   , "°C"      , 10. , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "β"   , "1"       , 1.  , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double PrModel::CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    return c_eq +
            x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) *
            CalcExpFactor(x->F(), x->T(), x->B(), gas);
}

void PrModel::CalculateDerivatives(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    const double z = PhysicalProperties::GetDryAirVolumeFraction(gas);
    const double d = PhysicalProperties::GetDiffusionCoefficientInWater(x->T(),
                                                                        gas);
    double d_to_beta;
    const double exp = CalcExpFactor(x->F(), x->T(), x->B(), gas, &d_to_beta);
    
    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        switch (parameter)
        {
        case Pr::A:
            derivative = z * exp;
            break;
            
        case Pr::F:
            derivative = -x->A() * z * exp * d_to_beta;
            break;
            
        case Pr::T:
            derivative += -x->A() * z * exp * x->F() * x->B() * d_to_beta / d *
                    PhysicalProperties::
                        GetDiffusionCoefficientInWaterDerivative(x->T(), gas);
            break;
            
        case Pr::B:
            derivative = -x->A() * z * exp * x->F() * d_to_beta * std::log(d);
            break;

        case Pr::OTHER:
            // Ableitung ist bereits richtig.
            break;

        default:
            derivative = 0;
            break;
        }
    }
}

std::string PrModel::GetModelName() const
{
    return NAME;
}

double PrModel::CalcExpFactor(double f, double t, double beta,
                              GasType gas, double* d_to_beta) const
{
    double d_to_b = std::pow(
            PhysicalProperties::GetDiffusionCoefficientInWater(t, gas),
            beta);
    if (d_to_beta) *d_to_beta = d_to_b;
    return std::exp(-f * d_to_b);
}
