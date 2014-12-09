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


#ifndef DERIVATIVECOLLECTOR_H
#define DERIVATIVECOLLECTOR_H

#include <Eigen/Core>

#include <vector>

//! Wird zum "Einsammeln" der benötigten Ableitungen bei der Berechnung der Ggw-Konzentration verwendet.
/*!
  Wird der CalculateDerivatives-Funktion einer CEqCalculationMethod übergeben. Die NobleFitFunction, die
  dieses Objekt übergibt muss zuvor angeben welche Ableitungen gewünscht sind und
  wo diese gespeichert werden sollen (SetDerivativesAndResultsVector). Die verschiedenen
  Excess-Air-Modelle und Gleichgewichts-Berechnungs-Methoden müssen Kindklassen implementieren. Die
  Standardimplementierung kann mit \ref generatehelperclasses.h erzeugt werden.
  */
class DerivativeCollector
{
public:

    ~DerivativeCollector() {}

    //! Legt fest, welche Ableitungen berechnet und wo diese gespeichert werden sollen.
    /*!
      \param indices Enthält die Indizes der Parameter nach denen abgeleitet werden soll. Diese
        entsprechen den Indizes die bei ihrer Anmeldung beim ParameterManager zurückgegeben wurden.
      \param results Referenz auf den Vektor, in dem die Ableitungen gespeichert werden sollen.
      \pre \p indices und \p results müssen die gleiche Größe haben.
      */
    void SetDerivativesAndResultsVector(Eigen::RowVectorXd& results, const std::vector<int>& indices);


protected:

    DerivativeCollector();


private:

    DerivativeCollector(const DerivativeCollector&);
    DerivativeCollector& operator=(const DerivativeCollector&);

    //! Aktualisiert die Informationen über die zu berechnenden Ableitungen und deren Speicherort.
    /*!
      Wird aufgerufen wenn Ableitungen oder Ergebnis-Vektor geändert werden.
      \param indices Enthält die Indizes der Parameter nach denen abgeleitet werden soll. Diese
        entsprechen den Indizes die bei ihrer Anmeldung beim ParameterManager zurückgegeben wurden.
      \param results Referenz auf den Vektor, in dem die Ableitungen gespeichert werden sollen.
      */
    virtual void UpdateDerivativeInformation(
        Eigen::RowVectorXd& results,
        const std::vector<int>& indices
        ) = 0;
};

#endif // DERIVATIVECOLLECTOR_H
