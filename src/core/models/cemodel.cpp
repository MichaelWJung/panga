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

#include "models/physicalproperties.h"

#include "cemodel.h"

const std::string CeModel::NAME = "CE";

CeModel::CeModel(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(manager,
                       "A", "ccSTP/g",  0.01,  0        , 0.05     , NOBLE_INF(),
                       "F", "1"      ,  0.  ,  0.05     , 1        , NOBLE_INF(),
                       "T", "°C"     , 10.  , -NOBLE_INF(), NOBLE_INF(), 2.       );
}

double CeModel::CalculateConcentration(
    double c_eq,
    std::shared_ptr<ParameterAccessor> parameters,
    GasType gas
) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    if (AreConstraintsApplied() && (x->F() < 0 || x->A() < 0 || x->F() > 30))
        return std::numeric_limits<double>::quiet_NaN();
    
    return c_eq +
           ((1. - x->F()) * x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas)) /
           (1. + x->F() * x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) / c_eq);
}

void CeModel::CalculateDerivatives(
    double c_eq,
    std::shared_ptr<ParameterAccessor> parameters,
    std::shared_ptr<DerivativeCollector> derivatives,
    GasType gas
) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    //Nenner der in jeder Ableitung vorkommt.
    double denominator = std::pow(1 + x->F() * x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) / c_eq, 2.);

    //Ableitung nach der Gleichgewichtskonzentration.
    double d_c_eq = 1. + (1. - x->F()) * x->F() *
                    std::pow(x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) / c_eq, 2.) /
                    denominator;


    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        switch (parameter)
        {
        case Ce::A:
            derivative = (1. - x->F()) * PhysicalProperties::GetDryAirVolumeFraction(gas) / denominator;
            break;

        case Ce::F:
            derivative = -x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) *
                         (1. + x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) / c_eq) /
                         denominator;
            break;

        case Ce::T:
        case Ce::OTHER:
            derivative *= d_c_eq;
            break;

        default:
            derivative = 0;
            break;
        }
    }
}

std::string CeModel::GetModelName() const
{
    return NAME;
}
