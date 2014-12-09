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

#include "diffusioncoefficientinair.h"

DiffusionCoefficientInAir::DiffusionCoefficientInAir(
        double T,
        double p,
        GasType gas) :
    gas_(gas == Gas::HE3 ? Gas::HE : gas),
    T_(T + 273.15),
    T_star_(T_ / combined_epsilon_over_kappa_.at(gas_)),
    p_(p / 1.01325)
{
}

double DiffusionCoefficientInAir::operator()() const
{
    return y_.at(gas_) * std::pow(T_, 1.5) / p_ /
           CalculateDiffusionCollisionIntegral();
}

double DiffusionCoefficientInAir::DeriveByP() const
{
    return -operator()() / p_;
}

double DiffusionCoefficientInAir::DeriveByT() const
{
    const double omega = CalculateDiffusionCollisionIntegral();
    const double d_omega_to_d_T =
            CalculateDerivativeOfDiffusionCollisionIntegral();
    return y_.at(gas_) * std::sqrt(T_) / p_ *
           (1.5 * omega - T_ * d_omega_to_d_T) /
           (omega * omega) / 1.01325;
}

double DiffusionCoefficientInAir::CalculateDiffusionCollisionIntegral() const
{
    return a_ / std::pow(T_star_, b_) +
           c_ / std::exp(d_ * T_star_) +
           e_ / std::exp(f_ * T_star_) +
           g_ / std::exp(h_ * T_star_);
}

double 
DiffusionCoefficientInAir::CalculateDerivativeOfDiffusionCollisionIntegral(
        ) const
{
    return -1. / combined_epsilon_over_kappa_.at(gas_) * (
           a_ * b_ / std::pow(T_star_, b_) +
           c_ * d_ / std::exp(d_ * T_star_) +
           e_ * f_ / std::exp(f_ * T_star_) +
           g_ * h_ / std::exp(h_ * T_star_));
}

std::map<GasType, double>
DiffusionCoefficientInAir::CalculateCombinedEpsilonOverKappa()
{
    std::map<GasType, double> values;
    for (GasType gas = Gas::begin; gas < Gas::end; ++gas)
        values[gas] = std::sqrt(epsilon_over_kappa_.at(gas) *
                                epsilon_over_kappa_air_);
    return values;
}

double DiffusionCoefficientInAir::CalculateMassMean(GasType gas)
{
    return 2. / (1. / molar_mass_air_ + 1. / molar_masses_.at(gas));
}

double DiffusionCoefficientInAir::CalculateMeanCollisionDiameter(GasType gas)
{
    return (collision_diameter_air_ + collision_diameters_.at(gas)) / 2.;
}

std::map< GasType, double > DiffusionCoefficientInAir::CalculateY()
{
    std::map<GasType, double> values;
    for (GasType gas = Gas::begin; gas < Gas::end; ++gas)
        values[gas] = 0.00266 / 
                      std::sqrt(CalculateMassMean(gas)) /
                      std::pow(CalculateMeanCollisionDiameter(gas), 2);
    return values;
}

const double DiffusionCoefficientInAir::a_ = 1.06036;
const double DiffusionCoefficientInAir::b_ = 0.15610;
const double DiffusionCoefficientInAir::c_ = 0.19300;
const double DiffusionCoefficientInAir::d_ = 0.47635;
const double DiffusionCoefficientInAir::e_ = 1.03587;
const double DiffusionCoefficientInAir::f_ = 1.52996;
const double DiffusionCoefficientInAir::g_ = 1.76474;
const double DiffusionCoefficientInAir::h_ = 3.89411;

// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const double DiffusionCoefficientInAir::epsilon_over_kappa_air_ = 97.0;

// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const std::map<GasType, double>
DiffusionCoefficientInAir::epsilon_over_kappa_ = {
        {Gas::HE,  10.22},
        {Gas::NE,  32.8 },
        {Gas::AR,  93.3 },
        {Gas::KR, 178.9 },
        {Gas::XE, 231.0 }};

// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const std::map<GasType, double>
DiffusionCoefficientInAir::combined_epsilon_over_kappa_ =
        CalculateCombinedEpsilonOverKappa();
        
// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const double DiffusionCoefficientInAir::molar_mass_air_ = 28.97; //UNSICHER!!!

// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const std::map<GasType, double>
DiffusionCoefficientInAir::molar_masses_ = {
        {Gas::HE,   4.002602},
        {Gas::NE,  20.1797  },
        {Gas::AR,  39.948   },
        {Gas::KR,  83.798   },
        {Gas::XE, 131.293   }};

// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const double DiffusionCoefficientInAir::collision_diameter_air_ = 3.620;
        
// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const std::map<GasType, double>
DiffusionCoefficientInAir::collision_diameters_ = {
        {Gas::HE, 2.551},
        {Gas::NE, 2.820},
        {Gas::AR, 3.542},
        {Gas::KR, 3.655},
        {Gas::XE, 4.047}};

// ACHTUNG, DIE REIHENFOLGE DIESER DEFINITIONEN AUF KEINEN FALL DURCHEINANDER
// BRINGEN, SONST GIBTS ÄRGER.
const std::map<GasType, double> DiffusionCoefficientInAir::y_ = CalculateY();