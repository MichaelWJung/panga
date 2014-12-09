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


#ifndef PRMODEL_H
#define PRMODEL_H

#include "excessairmodel.h"

#include "parametermanager.h"

#define NOBLE_PARAMETER_COUNT 4
#define NOBLE_PARAMETER_NAMES (A, F, T, B)
#define NOBLE_CLASS_PREFIX Pr
#define NOBLE_IS_EXCESS_AIR_MODEL
#include "generatehelperclasses.h"

//! PR-Modell zur Modellierung von Excess Air.
/*!
 * 
 * Aus Aeschbach-Hertig, W., Solomon, D.K., 2012. Noble gas thermometry in
 * groundwater hydrology, in: * Burnard, P. (Ed.), The noble gases as
 * geochemical tracers. Springer Verlag. Advances in Isotope Geochemistry.
 * \f[
     C^i(T,S,p,A,F_{PR},\beta) = C^i_{eq}(T,S,p) + Az_i\cdot
       e^{-F_{PR}\left(\frac{D_i}{D_{Ne}}\right)^\beta}
   \f]
 * Konzentrationen in ccSTP/g.
 * 
 * \f$p\f$: Luftdruck.\n
 * \f$T\f$: Temperatur.\n
 * \f$S\f$: Salinität.\n
 * \f$\beta\f$: Gas transfer parameter.\n
 * \f$A\f$: Concentration of dissolved excess air.\n
 * \f$F_{PR}\f$: Excess Air loss due to re-equilibration.\n
 * \f$z_i\f$: Volume fractions of the individual gases in dry air.
 */
class PrModel : public ExcessAirModel
{
    #include "generatehelpermethods.h"
public:

    //! Konstruktor.
    PrModel(std::shared_ptr<ParameterManager> manager);

    //! Destruktor.
    ~PrModel() {}

    double CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const;

    void CalculateDerivatives(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const;

    std::string GetModelName() const;

    static const std::string NAME;
    
private:
    double CalcExpFactor(double f, double t, double beta,
                         GasType gas, double* d_to_beta = nullptr) const;
};

#endif // PRMODEL_H
