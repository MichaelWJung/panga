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


#ifndef NOBLEPARAMETERMAP_H
#define NOBLEPARAMETERMAP_H

#include <boost/fusion/include/adapt_struct.hpp>

#include <QTextStream>

#include <Eigen/Core>

#include <vector>

#include "core/models/combinedmodelfactory.h"

#include "modelparameterconfig.h"

class FitParameterConfig;

//! Diese Klasse beschreibt, wie Modellparameter auf Fitparameter abgebildet werden.
class NobleParameterMap
{

public:

    //! Beschreibt, worauf ein einzelner Parameter abgebildet wird.
    struct Parameter
    {
        //! Wenn Wahr wird auf einen Fitparameter abgebildet. Sonst auf einen festen Wert.
        bool fitted;

        //! Index des Fitparameters auf den abgebildet wird.
        int target_index;

        //! Fester Wert auf den abgebildet wird.
        double value;
        
        bool operator==(const Parameter& other) const;
    };

    NobleParameterMap(std::shared_ptr<const CombinedModel> model,
                      const FitParameterConfig& fit_parameter_config,
                      const std::vector<ModelParameterConfigs>&
                              model_parameter_configs);

    //! Setzt Fitparameter auf feste Werte.
    /*!
      \param parameters Indices der Parameter, die auf feste Werte gesetzt werden sollen,
        sowie die Werte, auf die sie gesetzt werden sollen.
      */
    void FixParameters(std::vector<std::pair<int, double> > parameters);

    //! Setzt die Fitparameter wieder auf ihren ursprünglichen Zustand.
    /*!
      Macht das Setzen einzelner Parameter auf bestimmte Werte rückgängig.
      */
    void ResetParameters();

    //! Bestimmt die Modellparameter für die gegebenen Fitparametern.
    /*!
      \param fit_parameters Zu verwendende Fitparameter.
      \return Modellparameter.
      */
    std::vector<Eigen::VectorXd> MapParameterValues(const Eigen::VectorXd& fit_parameters) const;

    //! Gibt die Indizes der Parameter an, die gefittet werden.
    /*!
      \param sample_index Index der Probe.
      \return Vektor mit den Indizes der Parameter, die gefittet werden sollen.
      */
    const std::vector<int>& GetIndicesOfFittedParameters(int sample_index) const;

    //! Gibt die Anzahl der Proben zurück, für die Abbildungen definiert sind.
    /*!
      \return Anzahl der Proben.
      */
    unsigned GetNumberOfSamples() const;

    //! Gibt die Zahl der Fitparameter zurück.
    /*!
      \return Zahl der Fitparameter.
      */
    unsigned GetNumberOfFitParameters() const;
    
    bool operator==(const NobleParameterMap& other) const;

    //! Wird geworfen wenn in NobleParameterMap ein Fehler auftritt.
    class NobleParameterMapError;


private:

    //! Überprüft ob die Daten in \ref map_ sinnvoll sind.
    /*!
      Wirft einen NobleParameterMapError wenn die Daten nicht sinnvoll sind.
      */
    void ValidateParameterMap() const;

    //! Bestimmt und speichert die Indizes der Parameter, die gefittet werden sollen.
    void DetermineIndicesOfFittedParameters();

    //! Enthält die Abbildungs-Informationen aller Parameter.
    /*!
      Der äußere Vektor steht für die verschiedenen Proben.
      Der innere für die verschiedenen Modellparameter.
      */
    std::vector<std::vector<Parameter> > current_map_;

    std::vector<std::vector<Parameter> > original_map_;

    //! Vektor mit den Indizes der Parameter, die gefittet werden sollen.
    std::vector<std::vector<int> > indices_of_fitted_parameters_;

    class PairIsSecondNegative;
    class PairFirstGreater;
    class PairSecondLess;
};

class NobleParameterMap::NobleParameterMapError
{
public:
    NobleParameterMapError(std::string msg = "") : msg_(msg) {}
    std::string what()
    {
        return msg_;
    }
    
    std::string msg_;
};

BOOST_FUSION_ADAPT_STRUCT
(
    NobleParameterMap::Parameter,
    (bool, fitted)
    (int, target_index)
    (double, value)
)

#endif // NOBLEPARAMETERMAP_H
