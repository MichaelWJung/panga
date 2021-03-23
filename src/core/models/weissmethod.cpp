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


#include <boost/assign.hpp>

#include <stdexcept>

#include "physicalproperties.h"

#include "weissmethod.h"

const std::string WeissMethod::NAME = "WeissClever";

std::string WeissMethod::GetCEqMethodName() const   //WIP: Verwendet?
{
    return WeissMethod::NAME;
}

WeissMethod::WeissMethod(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(manager,
                       "p", "atm" ,  1., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
                       "S", "g/kg",  0., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
                       "T", "°C"  , 10., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double WeissMethod::CalculateConcentration(
    std::shared_ptr<ParameterAccessor> parameters,
    GasType gas
    ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    return CalculateConcentration(x->p(), x->S(), x->T(), gas);
}

void WeissMethod::CalculateDerivatives(
    std::shared_ptr<ParameterAccessor> parameters,
    std::shared_ptr<DerivativeCollector> derivatives,
    GasType gas
    ) const
{
    if (gas == Gas::XE)
        throw std::runtime_error("Xe equilibrium concentrations cannot be calculated using the"
                                 " Weiss method.");

    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    // Temperatur in Kelvin
    const double t_k = x->T() + 273.15;

    // Sättigungsdampfdruck.
    const double p_w = PhysicalProperties::CalcSaturationVaporPressure(x->T());

    // (p - pw) / (1 - pw)
    const double frac = (x->p() - p_w) / (1. - p_w);

    GasType gas_for_calculations = gas != Gas::HE3 ? gas : Gas::HE;
    
    const double exponential =
            CalcWeissExpFunction(x->T(), x->S(), gas_for_calculations) / 1000.;

    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        switch (parameter)
        {
        case Weiss::p:
            //Ableitung nach dem Druck.
            derivative = exponential / (1. - p_w);
            break;

        case Weiss::S:
            //Ableitung nach der Salinität.
            derivative = exponential * frac *
                          (s1[gas_for_calculations] +
                           s2[gas_for_calculations] * t_k +
                           s3[gas_for_calculations] * t_k * t_k);
            break;

        case Weiss::T:
            //Ableitung nach der Temperatur.
            derivative = exponential *

                          (frac *
                           //Ableitung des Exponenten
                           (-t2[gas_for_calculations] / t_k / t_k +
                             t3[gas_for_calculations] / t_k +
                             t4[gas_for_calculations] +
                            x->S() *
                            (s2[gas_for_calculations] +
                             s3[gas_for_calculations] * 2 * t_k))

                           +

                           (x->p() - 1.) / std::pow(1. - p_w, 2.) * //Ableitung von "frac"
                           PhysicalProperties::CalcSaturationVaporPressureDerivative(x->T()));
            break;

        case Weiss::OTHER:
        default:
            derivative = 0.;
            break;
        }
    }
    
    if (gas == Gas::HE3)
    {
        DERIVATIVE_LOOP(parameter, derivative, derivatives)
        {
            const double concentration =
                    CalculateConcentration(parameters, gas_for_calculations);
            const double r_eq = PhysicalProperties::CalcReq(x->T(), x->S());
            switch (parameter)
            {
                case Weiss::S:
                    derivative = derivative * r_eq + concentration *
                          PhysicalProperties::CalcReqDerivedByS(x->T(), x->S());
                    break;
                    
                case Weiss::T:
                    derivative = derivative * r_eq + concentration *
                          PhysicalProperties::CalcReqDerivedByT(x->T(), x->S());
                    break;
                    
                default:
                    derivative *= r_eq;
                    break;
            }
        }
    }
}

double WeissMethod::CalcWeissExpFunction(double t, double s, GasType gas)
{
    // Temperatur in Kelvin
    const double t_k = t + 273.15;

    return std::exp(t1[gas] +
                    t2[gas] / t_k +
                    t3[gas] * std::log(t_k) +
                    t4[gas] * t_k +
                    s * (s1[gas] +
                         s2[gas] * t_k +
                         s3[gas] * t_k * t_k));
}

double WeissMethod::CalculateConcentration(double p, double S, double T, GasType gas)
{
    if (gas == Gas::XE)
        throw std::runtime_error("Xe equilibrium concentrations cannot be calculated using the"
                                 " Weiss method.");

    // Sättigungsdampfdruck in atm.
    const double p_w = PhysicalProperties::CalcSaturationVaporPressure(T);

    // Partialdruck der trockenen Luft.
    const double p_dry = p - p_w;

    double concentration =
            CalcWeissExpFunction(T,
                                 S,
                                 gas != Gas::HE3 ? gas : Gas::HE) *
            p_dry / (1 - p_w) / 1000;

    if (gas == Gas::HE3)
        return concentration * PhysicalProperties::CalcReq(T, S);

    return concentration;
}

//Die Reihenfolge der Gase muss dem enum in "gas.h" entsprechen.
//Die Konstanten wurden umgerechnet, um das Teilen durch 100 zu vermeiden.
const std::vector<double> WeissMethod::t1 = boost::assign::list_of(  -808.2722264341 ) //Helium
                                                                  (  -819.4071883742 ) //Neon
                                                                  (  -846.9984052407 ) //Argon
                                                                  (  -455.6264185803 );//Krypton

const std::vector<double> WeissMethod::t2 = boost::assign::list_of( 21634.42         ) //Helium
                                                                  ( 22519.46         ) //Neon
                                                                  ( 25181.39         ) //Argon
                                                                  ( 15358.17         );//Krypton

const std::vector<double> WeissMethod::t3 = boost::assign::list_of(   139.2032       ) //Helium
                                                                  (   140.8863       ) //Neon
                                                                  (   145.2337       ) //Argon
                                                                  (    74.469        );//Krypton

const std::vector<double> WeissMethod::t4 = boost::assign::list_of(    -0.226202     ) //Helium
                                                                  (    -0.22629      ) //Neon
                                                                  (    -0.222046     ) //Argon
                                                                  (    -0.100189     );//Krypton

const std::vector<double> WeissMethod::s1 = boost::assign::list_of(    -0.044781     ) //Helium
                                                                  (    -0.127113     ) //Neon
                                                                  (    -0.038729     ) //Argon
                                                                  (    -0.011213     );//Krypton

const std::vector<double> WeissMethod::s2 = boost::assign::list_of(     0.00023541   ) //Helium
                                                                  (     0.00079277   ) //Neon
                                                                  (     0.00017171   ) //Argon
                                                                  (    -1.844e-05    );//Krypton

const std::vector<double> WeissMethod::s3 = boost::assign::list_of(    -3.4266e-07   ) //Helium
                                                                  (    -1.29095e-06  ) //Neon
                                                                  (    -2.1281e-07   ) //Argon
                                                                  (     1.1201e-07   );//Krypton
