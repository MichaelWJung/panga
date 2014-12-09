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

#include "odmodel.h"

const std::string OdModel::NAME = "OD";

OdModel::OdModel(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(
            manager,
            "A"   , "ccSTP/g", 0.01, -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "P_OD", "1"      , 1.  , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double OdModel::CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    if (AreConstraintsApplied() && (x->POD() < 1.0 || x->POD() > 1.26))
        return std::numeric_limits<double>::quiet_NaN();

    return c_eq * x->POD() +
            x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas);
}

void OdModel::CalculateDerivatives(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        switch (parameter)
        {
        case Od::A:
            derivative = PhysicalProperties::GetDryAirVolumeFraction(gas);
            break;
            
        case Od::POD:
            derivative = c_eq;
            break;

        case Od::OTHER:
            derivative *= x->POD();
            break;

        default:
            derivative = 0;
            break;
        }
    }
}

std::string OdModel::GetModelName() const
{
    return NAME;
}
