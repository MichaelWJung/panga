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


#ifndef EXCESSAIRMODEL_H
#define EXCESSAIRMODEL_H

#include <memory>

#include <vector>

#include "core/models/derivativecollector.h"
#include "core/models/parameteraccessor.h"

#include "core/misc/gas.h"

//! Interface für Excess-Air-Modelle.
/*!
  Kindklassen müssen die von ihnen zur Berechnung benötigten Parameter bei einem ParameterManager
  registrieren.
  */
class ExcessAirModel
{

public:
    ExcessAirModel() : apply_constraints_(false) {}

    //! Destruktor.
    virtual ~ExcessAirModel() {}

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

    //! Berechnet die Modellkonzentration für das gegebene Gas.
    /*!
      \param c_eq Gleichgewichtskonzentration im Wasser.
      \param parameters Zu verwendender Parametersatz, gekapselt in einem ParameterAccessor.
      \param gas Gas, für das die Modellkonzentration berechnet werden soll.
      \sa GetParameterAccessor
      */
    virtual double CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const = 0;

    //! Berechnet die Ableitungen der Modellkonzentration für das gegebene Gas.
    /*!
      \param c_eq Gleichgewichtskonzentration im Wasser.
      \param parameters Zu verwendender Parametersatz, gekapselt in einem ParameterAccessor.
      \param derivatives Stellt Informationen und Speicherorte für die zu berechnenden Ableitungen
        bereit. Die Ableitungen der Gleichgewichtskonzentration nach den jeweiligen Parametern sind an
        den Speicherorten zu finden an die die Ableitungen der Excess-Air-Modelle geschrieben werden
        sollen.
      \param gas Gas, für das die Ableitungen berechnet werden sollen.
      \sa GetDerivativeCollector
      */
    virtual void CalculateDerivatives(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const = 0;
        
    virtual std::string GetModelName() const = 0;
    
    //! \brief Wenn auf wahr gesetzt muss vom Modell nan zurückgegeben werden
    //! falls Parameterwerte außerhalb der Constraints angegeben werden.
    void SetApplyConstraints(bool apply) { apply_constraints_ = apply; }
    
    //! \brief Wenn auf wahr gesetzt muss vom Modell nan zurückgegeben werden
    //! falls Parameterwerte außerhalb der Constraints angegeben werden.
    bool AreConstraintsApplied() const { return apply_constraints_; }
    
private:
    //! \brief Wenn wahr sollen die Modelle nan zurückgeben falls die Parameter
    //! außerhalb der festgelegten Constraints sind.
    bool apply_constraints_;
};

#endif // EXCESSAIRMODEL_H
