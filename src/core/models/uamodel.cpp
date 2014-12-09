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

#include "uamodel.h"

const std::string UaModel::NAME = "UA";

UaModel::UaModel(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(manager,
                       "A", "ccSTP/g", 0.01, -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double UaModel::CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    return c_eq + x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas);
}

void UaModel::CalculateDerivatives(
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
        case Ua::A:
            derivative = PhysicalProperties::GetDryAirVolumeFraction(gas);
            break;

        case Ua::OTHER:
            // Nichts tun, enthält bereits die Ableitung von Ceq.
            break;

        default:
            derivative = 0;
            break;
        }
    }
}

std::string UaModel::GetModelName() const
{
    return NAME;
}
