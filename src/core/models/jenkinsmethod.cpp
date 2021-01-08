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

#include "jenkinsmethod.h"

const std::string JenkinsMethod::NAME = "Jenkins";

std::string JenkinsMethod::GetCEqMethodName() const //WIP: Verwendet?
{
    return JenkinsMethod::NAME;
}

JenkinsMethod::JenkinsMethod(std::shared_ptr<ParameterManager> manager)
{
    RegisterParameters(manager,
                       "p", "atm" ,  1., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
                       "S", "g/kg",  0., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF(),
                       "T", "°C"  , 10., -NOBLE_INF(), NOBLE_INF(), NOBLE_INF());
}

double JenkinsMethod::CalculateConcentration(
    std::shared_ptr<ParameterAccessor> parameters,
    GasType gas
    ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    return CalculateConcentration(x->p(), x->S(), x->T(), gas);
}

void JenkinsMethod::CalculateDerivatives(
    std::shared_ptr<ParameterAccessor> parameters,
    std::shared_ptr<DerivativeCollector> derivatives,
    GasType gas
    ) const
{
    DEFINE_PARAMETER_ACCESSOR(x, parameters);

    // Temperatur in Kelvin
    const double t_k = x->T() + 273.15;

    // Sättigungsdampfdruck.
    const double p_w = PhysicalProperties::CalcSaturationVaporPressure(x->T());

    // (p - pw) / (1 - pw)
    const double frac = (x->p() - p_w) / (1. - p_w);

    GasType gas_for_calculations = gas != Gas::HE3 ? gas : Gas::HE;
    
    const double exponential =
            CalcJenkinsExpFunction(x->T(), x->S(), gas_for_calculations) * PhysicalProperties::GetMolarVolume(gas) / 1000.;

    DERIVATIVE_LOOP(parameter, derivative, derivatives)
    {
        switch (parameter)
        {
        case Jenkins::p:
            //Ableitung nach dem Druck.
            derivative = exponential / (1. - p_w);
            break;

        case Jenkins::S:
            //Ableitung nach der Salinität.
            derivative = exponential * frac *
                          (s1[gas_for_calculations] +
                           s2[gas_for_calculations] * t_k +
                           s3[gas_for_calculations] * t_k * t_k +
                           s4[gas_for_calculations] * 2* x->S() );
            break;

        case Jenkins::T:
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

        case Jenkins::OTHER:
        default:
            derivative = 0.;
            break;
        }
    }
    
    if (gas == Gas::HE3) //WIP: durchgehen
    {
        DERIVATIVE_LOOP(parameter, derivative, derivatives)
        {
            const double concentration =
                    CalculateConcentration(parameters, gas_for_calculations);
            const double r_eq = PhysicalProperties::CalcReq(x->T(), x->S());
            switch (parameter)
            {
                case Jenkins::S:
                    derivative = derivative * r_eq + concentration *
                          PhysicalProperties::CalcReqDerivedByS(x->T(), x->S());
                    break;
                    
                case Jenkins::T:
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

double JenkinsMethod::CalcJenkinsExpFunction(double t, double s, GasType gas)
{
    // Temperatur in Kelvin
    const double t_k = t + 273.15;

    return std::exp(t1[gas] +
                    t2[gas] / t_k +
                    t3[gas] * std::log(t_k) +
                    t4[gas] * t_k +
                    s * (s1[gas] +
                         s2[gas] * t_k +
                         s3[gas] * t_k * t_k)+
                    s * s * s4[gas]);
}

double JenkinsMethod::CalculateConcentration(double p, double S, double T, GasType gas)
{
    // Sättigungsdampfdruck in atm.
    const double p_w = PhysicalProperties::CalcSaturationVaporPressure(T);

    // Partialdruck der trockenen Luft.
    const double p_dry = p - p_w;

    double concentration =
            CalcJenkinsExpFunction(T,
                                 S,
                                 gas != Gas::HE3 ? gas : Gas::HE) * //WIP: check HE3
            PhysicalProperties::GetMolarVolume(gas) / 1000. * //Umrechnung von mol/kg nach ccSTP/g
            p_dry / (1 - p_w);

    if (gas == Gas::HE3)
        return concentration * PhysicalProperties::CalcReq(T, S);

    return concentration;
}


//Die Reihenfolge der Gase muss dem enum in "gas.h" entsprechen.
//Das Teilen durch 100 wurde in die Konstanten t1, t2, t4, s2, s3 eingerechnet. (WIP: change)
const std::vector<double> JenkinsMethod::t1 = boost::assign::list_of( -826.322866779936) //Helium
                                                                    (-1319.35732470527 ) //Neon
                                                                    (-1058.82194230202 ) //Argon
                                                                    ( -445.738071028788) //Krypton
                                                                    ( -950.343306973085);//Xenon

const std::vector<double> JenkinsMethod::t2 = boost::assign::list_of(21759.91          ) //Helium
                                                                    (35262.01          ) //Neon
                                                                    (30543.47          ) //Argon
                                                                    (15356.54          ) //Krypton
                                                                    (29282.34          );//Xenon
 
const std::vector<double> JenkinsMethod::t3 = boost::assign::list_of(  140.7506        ) //Helium
                                                                    (  226.9676        ) //Neon
                                                                    (  180.5278        ) //Argon
                                                                    (   70.1969        ) //Krypton
                                                                    (  157.6127        );//Xenon
 
const std::vector<double> JenkinsMethod::t4 = boost::assign::list_of(   -0.2301954     ) //Helium
                                                                    (   -0.3713393     ) //Neon
                                                                    (   -0.279945      ) //Argon
                                                                    (   -0.0852524     ) //Krypton
                                                                    (   -0.2266895     );//Xenon
 
const std::vector<double> JenkinsMethod::s1 = boost::assign::list_of(   -0.038129      ) //Helium
                                                                    (   -0.06386       ) //Neon
                                                                    (   -0.066942      ) //Argon
                                                                    (   -0.049522      ) //Krypton
                                                                    (   -0.084915      );//Xenon
 
const std::vector<double> JenkinsMethod::s2 = boost::assign::list_of(    0.0001919     ) //Helium
                                                                    (    0.00035326    ) //Neon
                                                                    (    0.00037201    ) //Argon
                                                                    (    0.00024434    ) //Krypton
                                                                    (    0.00047996    );//Xenon
 
const std::vector<double> JenkinsMethod::s3 = boost::assign::list_of(   -2.6898E-07    ) //Helium
                                                                    (   -5.3258E-07    ) //Neon
                                                                    (   -5.6364E-07    ) //Argon
                                                                    (   -3.3968E-07    ) //Krypton
                                                                    (   -7.3595E-07    );//Xenon
 
const std::vector<double> JenkinsMethod::s4 = boost::assign::list_of(   -0.00000255    ) //Helium //WIP: Achtung, muss in JenkinsMethod auch drin sein
                                                                    (    0.0000128     ) //Neon
                                                                    (   -0.0000053     ) //Argon
                                                                    (    0.00000419    ) //Krypton
                                                                    (    0.00000669    );//Xenon