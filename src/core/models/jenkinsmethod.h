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


#ifndef JENKINSMETHOD_H
#define JENKINSMETHOD_H

#include <memory>

#include "ceqcalculationmethod.h"
#include "parametermanager.h"

#define NOBLE_PARAMETER_COUNT 3
#define NOBLE_PARAMETER_NAMES (p, S, T)
#define NOBLE_CLASS_PREFIX Jenkins
#define NOBLE_IS_CEQ_CALCULATION_METHOD
#include "generatehelperclasses.h"

//! Berechnung von Gleichgewichtskonzentrationen nach Jenkins. //WIP: Jenkins hier eintragen:
/*!
  Aus Kipfer et al. 2002, Noble Gases in Lakes and Ground Waters:
  \f[
    C^i_{eq}=exp\left(t^i_1+
                      t^i_2\cdot\frac{100}{T}+
                      t^i_3\cdot ln\left(\frac{T}{100}\right)+
                      t^i_4\cdot\frac{T}{100}+
                      S\cdot\left[s^i_1+
                                  s^i_2\cdot\frac{T}{100}+
                                  s^i_3\cdot\left(\frac{T}{100}\right)^2
                                  \right]
                      \right)\cdot
             \frac{p-e_w(T)}{(1-e_w(T))\cdot1000}
  \f]

  \f$C^i_{eq}\f$: Gleichgewichtskonzentration von i (He, Ne, Ar, Kr) in ccSTP/g.\n
  \f$e_w\f$: Sättigungsdampfdruck des Wassers.\n
  \f$p\f$: Luftdruck in atm.\n
  \f$T\f$: Temperatur in K.\n
  \f$S\f$: Salinität in g/kg.\n
  \f$t^i_j\f$: Temperaturkoeffizienten.\n
  \f$s^i_j\f$: Salinitätskoeffizienten.\n
  */
class JenkinsMethod : public CEqCalculationMethod
{
    #include "generatehelpermethods.h"
public:

    JenkinsMethod(std::shared_ptr<ParameterManager> manager);

    ~JenkinsMethod() {}

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
      \param gas Edelgas für das die Berechnung durchgeführt werden soll.
      \return Gleichgewichtskonzentration in ccSTP/g.
      */
    static double CalculateConcentration(double p,
                                         double S,
                                         double T,
                                         GasType gas);

    static const std::string NAME;
    
    std::string GetCEqMethodName() const; //WIP: Verwendet?

private:

    //! Berechnet die exp-Funktion aus der Jenkins-Formel.
    static double CalcJenkinsExpFunction(double t, double s, GasType gas);

    //! Jenkins coefficient.
    static const std::vector<double> t1;

    //! Jenkins coefficient.
    static const std::vector<double> t2;

    //! Jenkins coefficient.
    static const std::vector<double> t3;

    //! Jenkins coefficient.
    static const std::vector<double> t4;

    //! Jenkins coefficient.
    static const std::vector<double> s1;

    //! Jenkins coefficient.
    static const std::vector<double> s2;

    //! Jenkins coefficient.
    static const std::vector<double> s3;

    //! Jenkins coefficient.
    static const std::vector<double> s4;
};

#endif // JENKINSMETHOD_H
