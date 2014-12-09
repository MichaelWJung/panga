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


#ifndef ODMODEL_H
#define ODMODEL_H

#include "excessairmodel.h"

#include "parametermanager.h"

#define NOBLE_PARAMETER_COUNT 2
#define NOBLE_PARAMETER_NAMES (A, POD)
#define NOBLE_CLASS_PREFIX Od
#define NOBLE_IS_EXCESS_AIR_MODEL
#include "generatehelperclasses.h"

//! OD-Modell zur Modellierung von Excess Air.
/*!
 * 
 * Aus Aeschbach-Hertig, W., Solomon, D.K., 2012. Noble gas thermometry in
 * groundwater hydrology, in: * Burnard, P. (Ed.), The noble gases as
 * geochemical tracers. Springer Verlag. Advances in Isotope Geochemistry.
 * \f[
     C^i(T,S,p,A,P_{OD}) = C^i_{eq}(T,S,p)\cdot P_{OD} + Az_i
   \f]
 * Konzentrationen in ccSTP/g.
 * 
 * \f$p\f$: Luftdruck.\n
 * \f$T\f$: Temperatur.\n
 * \f$S\f$: Salinität.\n
 * \f$P_{OD}\f$: Overpressure factor.\n
 * \f$A\f$: Concentration of dissolved excess air.\n
 * \f$z_i\f$: Volume fractions of the individual gases in dry air.
 */
class OdModel : public ExcessAirModel
{
    #include "generatehelpermethods.h"
public:

    //! Konstruktor.
    OdModel(std::shared_ptr<ParameterManager> manager);

    //! Destruktor.
    ~OdModel() {}

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
};

#endif // ODMODEL_H
