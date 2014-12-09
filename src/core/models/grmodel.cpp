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

#include "diffusioncoefficientinair.cpp"
#include "physicalproperties.h"

#include "grmodel.h"

const std::string GrModel::NAME = "GR";

GrModel::GrModel(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(
            manager,
            "A"   , "ccSTP/g"  ,  0.01 , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "P_OD", "1"        ,  1.   , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "F_GR", "(s/cm²)^β",  1.   , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "β"   , "1"        ,  0.667, -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "T"   , "°C"       , 10.   , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
            "p"   , "atm"      ,  1.   , -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double GrModel::CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    return c_eq * x->POD() +
            x->A() * PhysicalProperties::GetDryAirVolumeFraction(gas) *
            CalcExpFactor(x->F(), x->B(), x->T(), x->p(), gas);
}

void GrModel::CalculateDerivatives(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    double z = PhysicalProperties::GetDryAirVolumeFraction(gas);
    double d;
    double d_to_beta;
    double exp =
            CalcExpFactor(x->F(), x->B(), x->T(), x->p(), gas, &d, &d_to_beta);
    double d_to_beta_minus1 = std::pow(d, x->B() - 1);
    
    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        switch (parameter)
        {
        case Gr::A:
            derivative = z * exp;
            break;
            
        case Gr::POD:
            derivative = c_eq;
            break;
            
        case Gr::F:
            derivative = -x->A() * z * exp * d_to_beta;
            break;
            
        case Gr::B:
            derivative = -x->A() * z * exp * x->F() * d_to_beta * std::log(d);
            break;
            
        case Gr::T:
            derivative = derivative * x->POD()
                    - x->A() * z * exp * x->F() * d_to_beta_minus1 * x->B() *
                     DiffusionCoefficientInAir(x->T(), x->p(), gas).DeriveByT();
            break;
                     
        case Gr::p:
            derivative = derivative * x->POD()
                    - x->A() * z * exp * x->F() * d_to_beta_minus1 * x->B() *
                     DiffusionCoefficientInAir(x->T(), x->p(), gas).DeriveByP();
            break;

        case Gr::OTHER:
            derivative *= x->POD();
            break;

        default:
            assert(false);
            derivative = 0;
            break;
        }
    }
}

std::string GrModel::GetModelName() const
{
    return NAME;
}

double GrModel::CalcExpFactor(
        double f,
        double beta,
        double T,
        double p,
        GasType gas,
        double* d_return,
        double* d_to_beta_return) const
{
    double d = DiffusionCoefficientInAir(T, p, gas)();
    double d_to_beta = std::pow(d, beta);
    if (d_return) *d_return = d;
    if (d_to_beta_return) *d_to_beta_return = d_to_beta;
    return std::exp(-f * d_to_beta);
}

