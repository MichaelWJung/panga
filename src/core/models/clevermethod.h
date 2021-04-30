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


#ifndef CLEVERMETHOD_H
#define CLEVERMETHOD_H

#include <memory>

#include "ceqcalculationmethod.h"
#include "parametermanager.h"

#define NOBLE_PARAMETER_COUNT 3
#define NOBLE_PARAMETER_NAMES (p, S, T)
#define NOBLE_CLASS_PREFIX Clever
#define NOBLE_IS_CEQ_CALCULATION_METHOD
#include "generatehelperclasses.h"

//! Berechnung von Gleichgewichtskonzentrationen nach Clever.
/*!
  Aus Kipfer et al. 2002, Noble Gases in Lakes and Ground Waters:
  \f[
    C^i_{eq}=\frac{X_i(T)}{M_{H_2O}}\cdot
           \frac{(p-e_w(T))\cdot z_i}{p_0}\cdot
           \frac{\rho(T,S=0)}{\rho(T,S)}\cdot
           V_i\cdot
           e^{-K_i(T)\cdot c_{NaCl}}
  \f]

  Aus Aeschbach-Hertig et al. 1999, Interpretation of dissolved atmospheric noble gases in natural
  waters:
  \f[
    c_{NaCl}=S\cdot\frac{\rho(p,T,S)}{M_{NaCl}}
  \f]

  \f$C^i_{eq}\f$: Gleichgewichtskonzentration von i (momentan nur Xe).\n
  \f$X_i\f$: \ref PhysicalProperties::CalcXeMoleFractionSolubility "Mole fraction solubility des Gases"
    (einheitenlos).\n
  \f$M_{H_2O}\f$: Molmasse von Wasser.\n
  \f$e_w\f$: \ref PhysicalProperties::CalcSaturationVaporPressure_Dickson "Sättigungsdampfdruck des Wassers".\n
  \f$z_i\f$: Volumen-Anteil des Gases in trockener Luft.\n
  \f$\rho(T,S)\f$: \ref PhysicalProperties::CalcWaterDensity "Dichte des Wassers".\n
  \f$V_i\f$: Molvolumen des Gases.\n
  \f$K_i\f$: \ref PhysicalProperties::CalcXeSaltingCoefficient "Salting Coefficient des Gases"
    in l/mol (Einheit unsicher).\n
  \f$c_{NaCl}\f$: Molare Konzentration von NaCl.\n
  \f$M_{NaCl}\f$: Molmasse von NaCl.\n
  \f$p\f$: Luftdruck.\n
  \f$T\f$: Temperatur.\n
  \f$S\f$: Salinität.
  */
class CleverMethod : public CEqCalculationMethod
{
    #include "generatehelpermethods.h"
public:

    CleverMethod(std::shared_ptr<ParameterManager> manager);

    ~CleverMethod() {}

    double CalculateConcentration(
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const;

    void CalculateDerivatives(
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const;


    //! Führt die eigentlich Berechnung der Konzentration durch.
    /*!
      \param p Druck in atm.
      \param S Salinität in g/kg.
      \param T Temperatur in °C.
      \return Gleichgewichtskonzentration in ccSTP/g.
      */
    static double CalculateConcentration(double p,
                                         double S,
                                         double T,
                                         GasType gas);

    static const std::string NAME;
    
    std::string GetCEqMethodName() const;

private:
    //! Volumen-Anteil von Xe in trockener Luft.
    static const double z_;
};

#endif // CLEVERMETHOD_H
