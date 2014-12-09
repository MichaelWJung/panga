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


#ifndef NOBLEFITFUNCTION_H
#define NOBLEFITFUNCTION_H

#include <memory>

#include <map>
#include <vector>

#include "core/misc/data.h"
#include "core/misc/gas.h"

#include "core/models/combinedmodelfactory.h"

#include "levenbergmarquardtfitter.h"
#include "nobleparametermap.h"

//! Wird zum Fitten eines Edelgas-Modells an gemessene Grundwasserkonzentrationen verwendet.
/*!
  Vor der Benutzung müssen Parameter mittels SetParameters festgelegt werden. Geschieht dies nicht
  kann für nichts garantiert werden.
  */
class NobleFitFunction
{

public:

    //! \todo Diesen Konstruktor näher beschreiben.
    /*!
     * \param concentrations Hier werden WAHRSCHEINLICH die Konzentrationen der am Fit
     * beteiligten Proben benötigt, und nicht alle!
     */
    NobleFitFunction(
        std::shared_ptr<const CombinedModel> model,
        const NobleParameterMap& parameter_map,
        const std::vector<std::map<GasType, Data> >& concentrations);

    ~NobleFitFunction();

    //! Legt die Parameter für die kommenden Berechnungen fest.
    /*!
      \param parameters Für kommende Berechnungen zu verwendende Parameter.
      */
    void SetParameters(const Eigen::VectorXd& parameters);

    //! Berechnet die Residuen für die übergebenen Parameter.
    /*!
      \param residuals Muss nach dem Aufruf den Residuen-Vektor beinhalten.
      */
    void CalcResiduals(Eigen::VectorXd& residuals) const;

    //! Berechnet die Jacobi-Matrix.
    /*!
      \param jacobi Muss nach dem Aufruf die Jacobi-Matrix beinhalten.
      */
    void CalcJacobian(Eigen::MatrixXd& jacobian) const;

    //! Gibt der Fitfunktion die Gelegenheit eventuelle weitere Ergebnisse zu speichern.
    void CompileResults(std::shared_ptr<FitResults> results);

    std::shared_ptr<NobleFitFunction> clone() const;

    //! Setzt Fitparameter auf feste Werte.
    /*!
      \param parameters Indices der Parameter, die auf feste Werte gesetzt werden sollen,
        sowie die Werte, auf die sie gesetzt werden sollen.
      */
    void FixParameters(std::vector<std::pair<int, double>> parameters);

    //! Setzt die Fitparameter wieder auf ihren ursprünglichen Zustand.
    /*!
      Macht das Setzen einzelner Parameter auf bestimmte Werte rückgängig.
      */
    void ResetParameters();

    //! Bestimmt die Zahl der vorhandenen Gaskonzentrationen.
    unsigned NumberOfConcentrations() const;

private:

    //! Initialisiert die Ableitungen der Modelle in models_ neu.
    void SetupDerivatives();

    //! Verwendetes Modell in mehrfacher Ausführung, je eins pro zu fittender Probe.
    std::vector<std::shared_ptr<CombinedModel> > models_;

    //! Verwendete NobleParameterMap. Zum Beschreiben der Abbildungen von Fit- auf Modellparameter.
    NobleParameterMap parameter_map_;

    //! Gaskonzentrationen der verschiedenen, für den Fit verwendeten Proben.
    std::vector<std::map<GasType, Data> > concentrations_;

    //! Modellparameter, die für die Berechnungen verwendet werden.
    std::vector<Eigen::VectorXd> parameters_;

    //! Gesamtzahl aller für den Fit verwendeten Gaskonzentrationen.
    const int n_concentrations_;
};

#endif // NOBLEFITFUNCTION_H
