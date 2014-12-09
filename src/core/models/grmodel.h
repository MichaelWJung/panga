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


#ifndef GRMODEL_H
#define GRMODEL_H

#include "excessairmodel.h"

#include "parametermanager.h"

#define NOBLE_PARAMETER_COUNT 6
#define NOBLE_PARAMETER_NAMES (A, POD, F, B, T, p)
#define NOBLE_CLASS_PREFIX Gr
#define NOBLE_IS_EXCESS_AIR_MODEL
#include "generatehelperclasses.h"

//! GR-Modell zur Modellierung von Excess Air.
/*!
 * 
 * Aus Aeschbach-Hertig, W., Solomon, D.K., 2012. Noble gas thermometry in
 * groundwater hydrology, in: * Burnard, P. (Ed.), The noble gases as
 * geochemical tracers. Springer Verlag. Advances in Isotope Geochemistry.
 * \f[
     C^i(T,S,p,A,P_{OD},F_{GR},\beta) = C^i_{eq}(T,S,p)\cdot P_{OD} + Az_i\cdot e^{-F_{GR}D_i^\beta}
   \f]
 * Konzentrationen in ccSTP/g.
 * 
 * \f$p\f$: Luftdruck.\n
 * \f$T\f$: Temperatur.\n
 * \f$S\f$: Salinität.\n
 * \f$P_{OD}\f$: Overpressure factor.\n
 * \f$\beta\f$: Gas transfer parameter.\n
 * \f$A\f$: Concentration of dissolved excess air.\n
 * \f$z_i\f$: Volume fractions of the individual gases in dry air.
 */
class GrModel : public ExcessAirModel
{
    #include "generatehelpermethods.h"
public:

    //! Konstruktor.
    GrModel(std::shared_ptr<ParameterManager> manager);

    //! Destruktor.
    ~GrModel() {}

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
    double CalcExpFactor(
            double f,
            double beta,
            double T,
            double p,
            GasType gas,
            double* d_return = nullptr,
            double* d_to_beta_return = nullptr) const;
};

#endif // GRMODEL_H
