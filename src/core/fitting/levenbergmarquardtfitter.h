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


#ifndef LEVENBERGMARQUARDTFITTER_H
#define LEVENBERGMARQUARDTFITTER_H

#include <Eigen/Dense>

#include <memory>

#include "fitparameterconfig.h"
#include "fitresults.h"

class NobleFitFunction;

//! Führt einen Levenberg-Marquardt-Least-Squares-Fit durch.
class LevenbergMarquardtFitter
{
public:
    //! Konstruktor.
    /*!
      \param func Zu verwendende Fitfunktion.
      */
    explicit LevenbergMarquardtFitter(std::shared_ptr<NobleFitFunction> func);

    //! Führt den Fit aus.
    /*!
      \param pconf Zu verwendende ParameterConfig. Enthält Initialwerte und Grenzen.
      \return Ergebnisse des Fits.
      */
    std::shared_ptr<FitResults> fit(std::shared_ptr<const FitParameterConfig> pconf) const;

    //! Erzeugt eine Kopie des Fitters.
    /*!
      Erzeugt dabei auch eine Kopie der Fitfunktion.
      \return Kopie des Fitters.
      */
    std::shared_ptr<LevenbergMarquardtFitter> clone() const;

    //! Gibt die verwendete Fitfunktion zurück.
    std::shared_ptr<NobleFitFunction> GetFitFunction();

private:
    
    //! Zeiger auf die verwendete Fitfunktion.
    std::shared_ptr<NobleFitFunction> func_;
};

#endif // LEVENBERGMARQUARDTFITTER_H
