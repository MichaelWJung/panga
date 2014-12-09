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


#ifndef CEQCALCULATIONMETHOD_H
#define CEQCALCULATIONMETHOD_H

#include <memory>

#include <vector>

#include "derivativecollector.h"
#include "parameteraccessor.h"

#include "core/misc/gas.h"

//! Interface einer Methode zur Berechnung der Gleichgewichtskonzentrationen von Edelgasen in Wasser.
/*!
  Kindklassen müssen die von ihnen zur Berechnung benötigten Parameter bei einem ParameterManager
  registrieren.
  */
class CEqCalculationMethod
{

public:

    //! Destruktor.
    virtual ~CEqCalculationMethod() {}

    //! Gibt einen ParameterAccessor zurück.
    /*!
      Dieser wird später, mit Werten für die Parameter befüllt, den Funktionen CalculateConcentration
      und CalculateDerivatives übergeben.
      */
    virtual std::shared_ptr<ParameterAccessor> GetParameterAccessor() const = 0;

    //! Gibt einen DerivativeCollector zurück.
    /*!
      Dieser wird, nachdem ihm mitgeteilt wurde welche Ableitungen berechnet werden sollen, an die
      Funktion CalculateDerivatives übergeben.
      */
    virtual std::shared_ptr<DerivativeCollector> GetDerivativeCollector() const = 0;

    //! Berechnet die Gleichgewichtskonzentration für das gegebene Gas.
    /*!
      \param parameters Zu verwendender Parametersatz, gekapselt in einem ParameterAccessor.
      \param gas Gas, für das die Gleichgewichtskonzentration berechnet werden soll.
      \return Berechnete Gleichgewichtskonzentration.
      \sa GetParameterAccessor
      */
    virtual double CalculateConcentration(
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const = 0;

    //! Berechnet die Ableitungen der Gleichgewichtskonzentrationen für das gegebene Gas.
    /*!
      \param parameters Zu verwendender Parametersatz, gekapselt in einem ParameterAccessor.
      \param derivatives Stellt Informationen und Speicherorte für die zu berechnenden Ableitungen
        bereit.
      \param gas Gas, für das die Ableitungen berechnet werden sollen.
      \sa GetDerivativeCollector
      */
    virtual void CalculateDerivatives(
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const = 0;
};

#endif // CEQCALCULATIONMETHOD_H
