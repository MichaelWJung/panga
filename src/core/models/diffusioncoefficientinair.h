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


#ifndef DIFFUSIONCOEFFICIENTINAIR_H
#define DIFFUSIONCOEFFICIENTINAIR_H

#include <map>

#include "misc/gas.h"

//! Berechnet Diffusionskoeffizienten von Gasen in Luft.
/*!
 * Aus Principles and Modern Applications of Mass Transfer Operations
 * (Jaime Benítez, 1948, second edition)
 * \f[
     D_{Ai} = \frac{0.00266T^{3/2}}{PM_{Ai}^{1/2}\sigma_{Ai}^2\Omega_D}
   \f]
   
   wobei:
   \f[
     M_{Ai} = 2 \left[\frac{1}{M_{air}}+\frac{1}{M_i}\right]^{-1}
   \f]
   
   \f$D_i\f$: Diffusionskoeffizient in \f$cm^2/s\f$.\n
   \f$M_{air}, M_i\f$: Molmassen von Luft und Gas in g.\n
   \f$T\f$: Temperatur in K.\n
   \f$p\f$: Druck in bar.\n
   \f$\sigma_{Ai}\f$: collision diameter, ein Lennard-Jones-Parameter in Å.\n
   \f$\Omega_D\f$: diffusion collision integral, dimensionslos.\n

   \f[
     \Omega_D = \frac{a}{(T^*)^b}+\frac{c}{exp(dT^*)}
     +\frac{e}{exp(fT^*)}+\frac{g}{exp(hT^*)}
   \f]
   mit Konstanten a bis h (siehe Quellcode).
   \f[
     T^*=\kappa T/\epsilon_{Ai}
   \f]
   \f[\sigma_{Ai}=\frac{\sigma_{air}+\sigma_i}{2}\f]
   \f[
     \frac{\epsilon_{Ai}}{\kappa}=
       \sqrt{\frac{\epsilon_{air}}{\kappa}\cdot\frac{\epsilon_i}{\kappa}}
   \f]
   \f$\epsilon_i/\kappa\f$ und \f$\sigma_i\f$ aus Anhang B des Buches.
 * 
 */
class DiffusionCoefficientInAir
{
public:
    /*!
     * \param T Temperatur in °C.
     * \param p Druck in atm.
     */
    DiffusionCoefficientInAir(double T, double p, GasType gas);
    
    //! Berechnet den Diffusionskoeffizienten.
    double operator()() const;
    
    //! Berechnet die Ableitung nach T.
    double DeriveByT() const;
    
    //! Berechnet die Ableitung nach P.
    double DeriveByP() const;
    
private:
    double CalculateDiffusionCollisionIntegral() const;
    double CalculateDerivativeOfDiffusionCollisionIntegral() const;
    
    GasType gas_;
    double T_;
    double T_star_;
    double p_;
    
    static std::map<GasType, double> CalculateCombinedEpsilonOverKappa();
    static double CalculateMassMean(GasType gas);
    static double CalculateMeanCollisionDiameter(GasType gas);
    static std::map<GasType, double> CalculateY();
    
    static const double a_;
    static const double b_;
    static const double c_;
    static const double d_;
    static const double e_;
    static const double f_;
    static const double g_;
    static const double h_;
    
    // Konstanten aus Anhang B des Buches.
    //! Lennard-Jones-Parameter ε/κ gemessen in K.
    static const double epsilon_over_kappa_air_;
    static const std::map<GasType, double> epsilon_over_kappa_;
    static const std::map<GasType, double> combined_epsilon_over_kappa_;
    
    static const double molar_mass_air_;
    static const std::map<GasType, double> molar_masses_;
    //! Collision diameter (Lennard-Jones-Parameter) σ gemessen in Å.
    static const double collision_diameter_air_;
    static const std::map<GasType, double> collision_diameters_;
    //! 0.00266 / (σ^2 * M ^0.5) * 1e-4
    static const std::map<GasType, double> y_;
};

#endif // DIFFUSIONCOEFFICIENTINAIR_H
