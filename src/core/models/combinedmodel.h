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


#ifndef COMBINEDMODEL_H
#define COMBINEDMODEL_H

#include <Eigen/Core>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "core/misc/gas.h"

#include "modelfactory.h"
#include "ceqmethodfactory.h"
#include "clevermethod.h"

#include "parametermanager.h"
#include "parameteraccessor.h"
#include "derivativecollector.h"

//! \brief Verbindet ein Excess-Air-Modell mit einer oder mehreren Methoden zur Berechnung der
//! Ggw-Konzentrationen.
/*!
  Die Parameter sind alle mit 0 initialisiert. Sie können mit SetParameters geändert werden.

  Vor der Berechnung von Ableitungen müssen diese mittels SetupDerivatives konfiguriert werden.
  */
class CombinedModel
{

public:

    //! Erzeugt mit dem mit der übergebenen Factory erstellten ExcessAirModel ein CombinedModel.
    CombinedModel(ModelFactory* factory, CEqMethodFactory* ceqmethod_factory);

    //! Setzt neue Parameter-Werte.
    /*!
      \param parameters Die Reihenfolge der Parameter muss der durch GetParametersInOrder vorgegebenen
        entsprechen.
      */
    void SetParameters(const Eigen::VectorXd& parameters);

    //! Legt fest, welche Ableitungen berechnet werden sollen.
    /*!
      \param indices Indizes der Parameter nach denen abgeleitet werden soll. Die Indizes entsprechen
        denen des von GetParametersInOrder zurückgegebenen Vektors.
      */
    void SetupDerivatives(const std::vector<int>& indices);

    //! Berechnet die Ggw-Konzentration für die mit SetParameters festgelegten Parameterwerte.
    /*!
      \param gas Gas, für das die Berechnung durchgeführt werden soll.
      \return Modellierte Ggw-Konzentration.
      */
    double CalculateEquilibriumConcentration(GasType gas);

    //! \brief Berechnet die modellierte Edelgas-Konzentration für die mit SetParameters festgelegten
    //! Parameterwerte.
    /*!
      \param gas Gas, für das die Berechnung durchgeführt werden soll.
      \return Modellierte Edelgas-Konzentration.
      */
    double CalculateConcentration(GasType gas);

    //! Berechnet die Ableitungen der Ggw-Konzentrationen für die mit SetParameters festgelegten Parameterwerte.
    /*!
      \param gas Gas, für das die Berechnung durchgeführt werden soll.
      \return Ableitungen nach den Parametern die mit SetupDerivatives vorgegeben wurden. Die
        Reihenfolge ist identisch. Die Referenz wird ungültig sobald diese Methode, CalculateDerivatives,
        SetParameterVector, SetupDerivatives oder ParametersChanged aufgerufen wird.
      */
    const Eigen::RowVectorXd& CalculateEquilibriumDerivatives(GasType gas);

    //! Berechnet die Ableitungen der Modellkonzentrationen für die mit SetParameters festgelegten Parameterwerte.
    /*!
      \param gas Gas, für das die Berechnung durchgeführt werden soll.
      \return Ableitungen nach den Parametern die mit SetupDerivatives vorgegeben wurden. Die
        Reihenfolge ist identisch. Die Referenz wird ungültig sobald diese Methode, CalculateEquilibriumDerivatives,
        SetParameterVector, SetupDerivatives oder ParametersChanged aufgerufen wird.
      */
    const Eigen::RowVectorXd& CalculateDerivatives(GasType gas);

    //! Gibt eine Liste aller Parameter, nach ihren Indizes geordnet, zurück.
    std::vector<ModelParameter> GetParametersInOrder() const;
    
    //! Gibt alle Parameternamen, nach ihren Indizes geordnet, zurück.
    std::vector<std::string> GetParameterNamesInOrder() const;
    
    //! Gibt ein Set mit allen Modellparameternamen zurück.
    std::set<std::string> GetParameterNames() const;

    //! Gibt den Index eines registrierten Parameters zurück.
    /*!
      Wirft ParameterNotFound, falls kein Parameter dieses Namens gefunden wurde.
      */
    unsigned GetParameterIndex(const std::string& name) const;

    //! Erzeugt eine Kopie des Modells.
    std::shared_ptr<CombinedModel> clone() const;

    //! Wird geworfen, falls der gesuchte Parameter nicht gefunden werden kann.
    class ParameterNotFound {};
    
    //! Gibt den Namen des verwendeten ExcessAirModels zurück.
    std::string GetExcessAirModelName() const;

    //! Gibt den Namen der verwendeten CEqMethod zurück.
    std::string GetCEqMethodName() const;

    //! \brief Wenn auf wahr gesetzt wird vom Modell nan zurückgegeben falls
    //! Parameterwerte außerhalb der Constraints angegeben werden.
    void SetApplyConstraints(bool apply);
    
    //! \brief Wenn auf wahr gesetzt wird vom Modell nan zurückgegeben falls
    //! Parameterwerte außerhalb der Constraints angegeben werden.
    bool AreConstraintsApplied() const;

    
private:

    //! ModelFactory wird gespeichert um das CombinedModel leicht klonen zu können.
    ModelFactory*
    factory_;

    //! ModelFactory wird gespeichert um das CombinedModel leicht klonen zu können.
    CEqMethodFactory*
    ceqmethod_factory_;

    //! ParameterManager der zur Konfiguration der Modelle verwendet wird.
    std::shared_ptr<ParameterManager> manager_;

    //! Verwendetes ExcessAirModel
    std::shared_ptr<ExcessAirModel> model_;

    //! Verwendete CEqCalculationMethod
    std::shared_ptr<CEqCalculationMethod> ceqmethod_;

    //! Clever-Methode zur Berechnung der Gleichgewichtskonzentrationen von Xe.
    std::shared_ptr<CleverMethod> clever_;

    //! ParameterAccessor des \ref ExcessAirModel "ExcessAirModels".
    std::shared_ptr<ParameterAccessor>  model_accessor_;

    //! ParameterAccessor der \ref CEqCalculationMethod "CEqCalculationMethods".
    std::shared_ptr<ParameterAccessor>  ceqmethod_accessor_;

    //! ParameterAccessor der CleverMethod.
    std::shared_ptr<ParameterAccessor> clever_accessor_;

    //! DerivativeCollector des \ref ExcessAirModel "ExcessAirModels".
    std::shared_ptr<DerivativeCollector>  model_collector_;

    //! DerivativeCollector des \ref CEqCalculationMethod "CEqCalculationMethods".
    std::shared_ptr<DerivativeCollector>  ceqmethod_collector_;

    //! DerivativeCollector der CleverMethod.
    std::shared_ptr<DerivativeCollector> clever_collector_;

    //! Hier werden die Parameter zwischengespeichert.
    Eigen::VectorXd parameters_;

    //! Hier werden berechnete Ableitungen gespeichert.
    Eigen::RowVectorXd derivatives_;

    //! Zwischenspeicher für Gleichgewichtskonzentrationen.
    std::map<GasType, double> cached_concentrations_;
};

#endif // COMBINEDMODEL_H
