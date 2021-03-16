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

#include "physicalproperties.h"

const double PhysicalProperties::c1_ = 1.80960162;
const double PhysicalProperties::c2_ = 0.080060884;
const double PhysicalProperties::c3_ = 0.00412;

const std::map<GasType, std::vector<double> > PhysicalProperties::virial_coefficient_parameters_ = {
    {Gas::HE, std::vector<double>({  12.44, -  1.25             })},
    {Gas::NE, std::vector<double>({  10.80, -  7.50, - 0.4      })},
    {Gas::AR, std::vector<double>({- 16.00, - 60.00, - 9.7, -1.5})},
    {Gas::KR, std::vector<double>({- 51.00, -118.00, -29.0, -5.0})},
    {Gas::XE, std::vector<double>({-130.00, -262.00, -87.0      })},
};

const std::map<GasType, double> PhysicalProperties::molar_volumes_ =
    PhysicalProperties::CalculateMolarVolumes(PhysicalProperties::virial_coefficient_parameters_);

const double PhysicalProperties::molar_volume_ideal_gas_ = 22414.1;

// Molare Volumen basierend auf Dymond and Smith, 1980
// calculated using the second Virial coefficient approximation
// verwendet für die Umrechnung der Gleichgewichtskonzentrationen
const std::map<GasType, double> PhysicalProperties::molar_volumes_new = { 
    {Gas::HE, 22425.8703182828}, // Porcelli: 22436.4 
    {Gas::NE, 22424.8703182828}, //           22421.7
    {Gas::AR, 22392.5703182828}, //           22386.5
    {Gas::KR, 22352.8703182828}, //           22351.2
    {Gas::XE, 22256.9703182828}, //           22280.4
};
double PhysicalProperties::GetMolarVolume(GasType gas)
{
    return PhysicalProperties::molar_volumes_new.find(gas)->second;
}



//! \todo Eine Lösung finden für das doppelte definieren des Xenon-Werts.
const std::map<GasType, double> PhysicalProperties::dry_air_volume_fractions_ = {
    {Gas::HE, 5.24e-6 },
    {Gas::NE, 1.818e-5},
    {Gas::AR, 9.34e-3 },
    {Gas::KR, 1.14e-6 },
    {Gas::XE, 8.7e-8  }, //Vorsicht, diese Größe steht auch in clevermethod.cpp!
    {Gas::HE3, 5.24e-6 * 1.384e-6}
};

const std::map<GasType, std::pair<double, double>>
        PhysicalProperties::diffusion_coefficients_in_water_constants_ = {
    {Gas::HE, { 818, 11700}},
    {Gas::NE, {1608, 14840}},
    {Gas::AR, {2233, 16680}},
    {Gas::KR, {6393, 20200}},
    {Gas::XE, {9007, 21610}}
};

// const double PhysicalProperties::R = 0.082058;
const double PhysicalProperties::R = 8.3143;

//Benson and Krause 1980; Kipfer et al. 2002
//Coefficients for the calculation of 3He/4He
const double PhysicalProperties::r1_ = -0.0299645;
const double PhysicalProperties::r2_ = 19.8715;
const double PhysicalProperties::r3_ = -1833.92;
const double PhysicalProperties::r4_ = 0.000464;
const double PhysicalProperties::r5_ = 1.384;

double PhysicalProperties::CalcSaturationVaporPressure_Gill(double T_c)
{
    return std::exp( (c1_ + c2_ * T_c) / (1 + c3_ * T_c) )
            / 1013.25;
}
double PhysicalProperties::CalcSaturationVaporPressure_Dickson(double T_c, double S)
{
    //Calculate temperature in Kelvin and modified temperature for Chebyshev polynomial
    double temp_K = T_c+273.15;
    double temp_mod = 1-temp_K/647.096;

    //Calculate value of Wagner polynomial
    double Wagner = -7.85951783*temp_mod +1.84408259*pow(temp_mod, 1.5) -11.7866497*pow(temp_mod, 3) +22.6807411*pow(temp_mod, 3.5) -15.9618719*pow(temp_mod, 4) +1.80122502*pow(temp_mod, 7.5);

    //Vapor pressure of pure water in kiloPascals and mm of Hg
    double vapor_0sal_kPa = exp(Wagner * 647.096 / temp_K) * 22.064 * 1000;

    //Correct vapor pressure for salinity
    double molality = 31.998 * S /(1e3-1.005*S);
    double osmotic_coef = 0.90799 -0.08992*(0.5*molality) +0.18458*pow(0.5*molality,2) -0.07395*pow(0.5*molality,3) -0.00221*pow(0.5*molality,4);
    double vapor_press_kPa = vapor_0sal_kPa * exp(-0.018 * osmotic_coef * molality);

    //Convert to atm
    double vapor_press_atm = vapor_press_kPa/101.32501;

    // double oldpressure = CalcSaturationVaporPressure(T_c);
    // printf("%.20g\n", vapor_press_atm);
    // printf("%.20g\n", oldpressure);

    return vapor_press_atm;
}

double PhysicalProperties::CalcSaturationVaporPressureDerivative_Gill(double T_c)
{
    return CalcSaturationVaporPressure_Gill(T_c) *
            (c2_ - c1_ * c3_) / std::pow(1 + c3_ * T_c, 2.);
}
double PhysicalProperties::CalcSaturationVaporPressureDerivedByT_Dickson(double T_c, double S)
{
    //Calculate temperature in Kelvin and modified temperature for Chebyshev polynomial
    double temp_K = T_c+273.15;
    double temp_mod = 1-temp_K/647.096;

    //Calculate value of Wagner polynomial
    double Wagner = -7.85951783*temp_mod +1.84408259*pow(temp_mod, 1.5) -11.7866497*pow(temp_mod, 3) +22.6807411*pow(temp_mod, 3.5) -15.9618719*pow(temp_mod, 4) +1.80122502*pow(temp_mod, 7.5);
    double WagnerDerivedByTmod = -7.85951783+ 1.5* 1.84408259*pow(temp_mod, 0.5) - 3* 11.7866497*pow(temp_mod, 2) + 3.5* 22.6807411*pow(temp_mod, 2.5) - 4* 15.9618719*pow(temp_mod, 3) + 7.5* 1.80122502*pow(temp_mod, 6.5);
    double WagnerDerivedByT = - 1/647.096 * WagnerDerivedByTmod;
    return CalcSaturationVaporPressure_Dickson(T_c, S)* (WagnerDerivedByT * 647.096 / temp_K - Wagner * 647.096 / pow(temp_K,2));
}
double PhysicalProperties::CalcSaturationVaporPressureDerivedByS_Dickson(double T_c, double S)
{
    //Correct vapor pressure for salinity
    double molality = 31.998 * S /(1e3 - 1.005*S);
    double molality_DerivedByS = 31.998 /(1e3 - 1.005*S) + 31.998 * S / pow(1e3 - 1.005*S, 2) * 1.005;
    double osmotic_coef = 0.90799 -0.08992*(0.5*molality) +0.18458*pow(0.5*molality,2) -0.07395*pow(0.5*molality,3) -0.00221*pow(0.5*molality,4);
    double osmotic_coef_DerivedByMolality = -0.08992*(0.5) + 2*0.5* 0.18458*pow(0.5*molality,1) - 3*0.5* 0.07395*pow(0.5*molality,2) - 4*0.5* 0.00221*pow(0.5*molality,3);
    double osmotic_coef_DerivedByS = molality_DerivedByS * osmotic_coef_DerivedByMolality;

    return CalcSaturationVaporPressure_Dickson(T_c, S)* (-0.018 * (osmotic_coef_DerivedByS * molality + osmotic_coef * molality_DerivedByS ));
}

double PhysicalProperties::CalcWaterDensity(double p, double S, double T_c)
{
    //Umwandeln von atm in "bar über 1".
    p = p * 1.01325 - 1;
    const double& T = T_c;
    const double T2 = T * T;
    const double T3 = T2 * T;
    const double T4 = T3 * T;
    const double p2 = p * p;
    const double S32 = std::pow(S, 0.3e1 / 0.2e1);

    return (0.999842594e3 + 0.6793952e-1 * T - 0.9095290e-2 * T2 + 0.1001685e-3 *
            T3 - 0.1120083e-5 * T4 + 0.6536332000e-8 * T4 * T +

            S * (0.824493e0 - 0.40899e-2 * T + 0.76438e-4 * T2 - 0.8246700000e-6 *
                 T3 + 0.5387500000e-8 * T4) +

            S32 * (-0.572466e-2 + 0.10227e-3 * T - 0.16546e-5 * T2) +

            0.48314e-3 * S * S) /


           (0.1e1 - p /

            (0.1965221e5 + 0.1484206e3 * T - 0.2327105e1 * T2 +
             0.1360477e-1 * T3 - 0.5155288e-4 * T4 +

             S * (0.546746e2 - 0.603459e0 * T + 0.109987e-1 * T2 -
                  0.61670e-4 * T3) +

             S32 * (0.7944e-1 + 0.16483e-1 * T - 0.53009e-3 * T2) +

             p * (0.3239908e1 + 0.143713e-2 * T + 0.116092e-3 * T2 -
                  0.5779050000e-6 * T3) +

             p * S * (0.22838e-2 - 0.10981e-4 * T - 0.16078e-5 * T2) +

             0.191075e-3 * p * S32 +

             p2 * (0.850935e-4 - 0.612293e-5 * T + 0.5278700000e-7 * T2) +

             p2 * S * (-0.9934800000e-6 + 0.2081600000e-7 * T + 0.9169700000e-9 * T2)));
}

double PhysicalProperties::CalcXeMoleFractionSolubility(double T_c)
{
    const double T_k = T_c + 273.15;
    return std::exp(-0.2012272464e3 + 0.105210e5 / T_k + 0.274664e2 * std::log(T_k));
}

double PhysicalProperties::CalcXeSaltingCoefficient(double T_c)
{
    const double T_k = T_c + 273.15;
    return -0.4431009868e2 + 0.218772e4 / T_k + 0.65527e1 * std::log(T_k);
}

double PhysicalProperties::ConvertToMole(double ccstp, GasType gas) //WIP: Verwendung molares Volumen
{
    return ccstp / molar_volumes_.find(gas)->second;
}

double PhysicalProperties::ConvertToMolePerLiter(double ccstp_g, double p, double S, double T_c, GasType gas)
{
    return ccstp_g * CalcWaterDensity(p, S, T_c) / molar_volumes_.find(gas)->second; //WIP: Verwendung molares Volumen
}

std::map<GasType, double> PhysicalProperties::CalculateMolarVolumes(
    const std::map<GasType, std::vector<double> > &a //WIP: virial_coefficient_parameters_
    )
{
    std::map<GasType, double> ret;

    for (std::map<GasType, std::vector<double> >::const_iterator it = a.begin(); it != a.end(); ++it)
    {
        double b = 0;
        unsigned i = 0;
        for (std::vector<double>::const_iterator jt = it->second.begin();
             jt != it->second.end();
             ++jt, ++i)
            b += *jt * std::pow(298.15 / 273.15 - 1, i);

        ret[it->first] = b + molar_volume_ideal_gas_;
    }

    return ret;
}

double PhysicalProperties::GetDryAirVolumeFraction(GasType gas)
{
    return dry_air_volume_fractions_.find(gas)->second;
}

double PhysicalProperties::GetDiffusionCoefficientInWater(double T_c,
                                                          GasType gas)
{
    if (gas == Gas::NE) return 1.;
    GasType gas_for_calculation = gas != Gas::HE3 ? gas : Gas::HE;
    const double T_k = T_c + 273.15;
    const auto a =
            diffusion_coefficients_in_water_constants_.at(gas_for_calculation);
    const double a1 = a.first;
    const double a2 = a.second;
    const auto b = diffusion_coefficients_in_water_constants_.at(Gas::NE);
    const double b1 = b.first;
    const double b2 = b.second;
    const double coefficient = a1 / b1 * std::exp((b2 - a2) / (R * T_k));
    if (gas == Gas::HE3)
        return coefficient * std::sqrt(4. / 3.);
    return coefficient;
}

double PhysicalProperties::GetDiffusionCoefficientInWaterDerivative(double T_c,
                                                                    GasType gas)
{
    if (gas == Gas::NE) return 0.;
    GasType gas_for_calculation = gas != Gas::HE3 ? gas : Gas::HE;
    const double T_k = T_c + 273.15;
    const auto a =
            diffusion_coefficients_in_water_constants_.at(gas_for_calculation);
    const double a2 = a.second;
    const auto b = diffusion_coefficients_in_water_constants_.at(Gas::NE);
    const double b2 = b.second;
    const double derivative =
            GetDiffusionCoefficientInWater(T_c, gas_for_calculation) *
                (a2 - b2) / (R * T_k * T_k);
    if (gas == Gas::HE3)
        return derivative * std::sqrt(4. / 3.);
    return derivative;
}

double PhysicalProperties::CalcReq(double t, double s)
{
    const double t_k = t + 273.15;
    const double a = std::exp((r1_ + r2_ / t_k + r3_ / t_k / t_k) *
            (1 +  r4_ * s));
    return r5_ / a / 1e6;
}

double PhysicalProperties::CalcReqDerivedByT(double t, double s)
{
    const double t_k = t + 273.15;
    const double r_eq = CalcReq(t, s);
    return r_eq * (1 + r4_ * s) * (2 * r3_ / (t_k*t_k*t_k) + r2_ / (t_k*t_k));
}

double PhysicalProperties::CalcReqDerivedByS(double t, double s)
{
    const double t_k = t + 273.15;
    const double r_eq = CalcReq(t, s);
    return -r_eq * (r1_ + r2_ / t_k + r3_ / (t_k*t_k)) * r4_;
}
