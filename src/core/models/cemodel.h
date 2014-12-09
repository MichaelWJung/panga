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


#ifndef CEMODEL_H
#define CEMODEL_H

#include "parametermanager.h"

#include "excessairmodel.h"

#define NOBLE_PARAMETER_COUNT 3
// T wird nicht für das Modell verwendent, nur um den normalen T-Bereich
// einzuschränken
#define NOBLE_PARAMETER_NAMES (A, F, T)
#define NOBLE_CLASS_PREFIX Ce
#define NOBLE_IS_EXCESS_AIR_MODEL
#include "generatehelperclasses.h"

//! CE-Modell zur Modellierung von Excess Air.
/*!
  Aus Aeschbach-Hertig et al., 2000, Nature, Palaeotemperature reconstruction from noble gases in ground
  water taking into account equilibration with entrapped air:
  \f[
    C^i(T,S,p,A,F) = C^i_{eq}(T,S,p)+
                     \frac{(1-F)\cdot Az_i}
                          {1+\frac{FAz_i}{C^i_{eq}(T,S,p)}}
  \f]

  Konzentrationen in ccSTP/g.

  \f$p\f$: Luftdruck.\n
  \f$T\f$: Temperatur.\n
  \f$S\f$: Salinität.\n
  \f$A\f$: Initial amount of entrapped air per unit mass of water.\n
  \f$F\f$: F is the ratio of two parameters with a clear physical meaning: v, the ratio of the entrapped
    gas volumes in final and initial state, and q, the ratio of the dry gas pressure in the trapped gas
    to that in the free atmosphere.\n
  \f$z_i\f$: Volume fractions of the individual gases in dry air.
  */
class CeModel : public ExcessAirModel
{
#include "generatehelpermethods.h"
public:

    //! Konstruktor.
    CeModel(std::shared_ptr<ParameterManager> manager);

    //! Destruktor.
    ~CeModel() {}

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

#endif // CEMODEL_H
