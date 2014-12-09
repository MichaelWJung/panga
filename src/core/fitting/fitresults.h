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


#ifndef FITRESULTS_H
#define FITRESULTS_H

#include <Eigen/Core>

#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

#include <map>
#include <vector>

#include "core/misc/data.h"
#include "core/misc/gas.h"

#include "eigen_lm_includes.h"

//! Bündelt die Fitergebnisse
class FitResults
{

public:

    FitResults() : chi_square(-1.), n_iterations(0) {}

    virtual ~FitResults() {}

    static std::string GetExitFlagAsString(Eigen::LM::Status flag);
    std::string GetExitFlagAsString() const;
    virtual double CorrelationCoefficient(unsigned row, unsigned col) const;

    //! χ² des Fits.
    double chi_square;

    //! Bester gefundener Parametersatz.
    Eigen::VectorXd best_estimate;

    //! Standardabweichungen der Parameter.
    Eigen::VectorXd deviations;

    //! Kovarianz-Matrix.
    Eigen::MatrixXd covariance_matrix;

    //! Residuen des Fits.
    Eigen::VectorXd residuals;

    //! \brief Enthält die Gastypen, die im Vektor residuals an der gleichen
    //! Stelle zu finden sind
    std::vector<GasType> residual_gases;

    //! Zahl der benötigten Iterationsschritte.
    int n_iterations;

    //! Zahl der Freiheitsgrade des Fits.
    int degrees_of_freedom;

    //! Gibt an, wodurch der Fit beendet wurde.
    Eigen::LM::Status exit_flag;

    std::vector<std::map<GasType, Data>> model_concentrations;
    std::vector<std::map<GasType, Data>> equilibrium_concentrations;
    std::vector<std::map<GasType, Data>> measured_concentrations;
};

#endif // FITRESULTS_H