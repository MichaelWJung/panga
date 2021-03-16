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



#include <cmath>
#include <iomanip>
#include <limits>

#include "physicalproperties.h"

#include "clevermethod.h"

const std::string CleverMethod::NAME = "Weiss,Clever(Xe)";  //WIP: Neu, verwendet?

std::string CleverMethod::GetCEqMethodName() const  //WIP: Neu, verwendet?
{
    return CleverMethod::NAME;
}

CleverMethod::CleverMethod(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(manager,
                       "p", "atm" ,  1., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
                       "S", "g/kg",  0., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
                       "T", "°C"  , 10., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double CleverMethod::CalculateConcentration(
    std::shared_ptr<ParameterAccessor> parameters,
    GasType gas
    ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    return CalculateConcentration(x->p(), x->S(), x->T(), gas);
}

void CleverMethod::CalculateDerivatives(
    std::shared_ptr<ParameterAccessor> parameters,
    std::shared_ptr<DerivativeCollector> derivatives,
    GasType gas
    ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    double concentration = CalculateConcentration(x->p(), x->S(), x->T(), gas);

    
    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        // numerisch ableiten
        
        double p = x->p();
        double S = x->S();
        double T = x->T();
        double* mod = 0;

        // den zu variierenden Parameter bestimmen
        switch (parameter)
        {
        case Clever::p:
            mod = &p;
            break;

        case Clever::S:
            mod = &S;
            break;

        case Clever::T:
            mod = &T;
            break;

        case Clever::OTHER:
        default:
            derivative = 0.;
            continue;
        }

        double h;

        if (std::abs(*mod) >
            std::pow(static_cast<double>(std::numeric_limits<double>::radix),
                     std::numeric_limits<double>::min_exponent + 20))
            h = std::sqrt(std::numeric_limits<double>::epsilon()) * *mod;
        else
            // 1e-7 scheint bei den hier verwendeten Parametern eine vernünftige Größe zu sein.
            h = 1e-7;
                //std::pow(static_cast<double>(std::numeric_limits<double>::radix),
                //         std::numeric_limits<double>::min_exponent + 10);

        // this ensures that the difference of mod+h and mod
        // is exactly h (floating point precision).
        volatile double temp = *mod + h;
        h = temp - *mod;

        // Parameter variieren
        *mod = temp;

        // Differenzenquotient berechnen
        derivative = (CalculateConcentration(p, S, T, Gas::XE) - concentration) / h;
    }
}

double CleverMethod::CalculateConcentration(double p, double S, double T, GasType gas)
{
    if (gas != Gas::XE)
        throw std::runtime_error("Only Xe equilibrium concentrations can be calculated using "
                                 "the Clever method. Other gases are not implemented.");

    return PhysicalProperties::CalcXeMoleFractionSolubility(T) /
           // Molmasse von Wasser [g/mol]
           18.016 *

           PhysicalProperties::GetMolarVolume(gas) *

            // Partialdruck der trockenen Luft                      * Xe-Anteil
           (p - PhysicalProperties::CalcSaturationVaporPressure_Dickson(T, S)) * z_ *

           PhysicalProperties::CalcWaterDensity(1., 0., T) /
           PhysicalProperties::CalcWaterDensity(1., S , T) *

           std::exp(-S * PhysicalProperties::CalcWaterDensity(p, S, T) * .001 /
                    // Molmasse von NaCl [g/mol]
                    58.443 * PhysicalProperties::CalcXeSaltingCoefficient(T));
}

//Vorsicht, diese Größe steht auch in physicalproperties.cpp!
const double CleverMethod::z_ = 8.7e-8;
