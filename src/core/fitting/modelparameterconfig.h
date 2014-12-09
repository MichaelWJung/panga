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


#ifndef MODELPARAMETERCONFIG_H
#define MODELPARAMETERCONFIG_H

#include <string>
#include <vector>

//! Beschreibt einen Modellparameter.
/*!
 * Gibt an, ob dieser gefittet oder auf einen festen Wert gesetzt wird.
 */
struct ModelParameterConfig
{
    ModelParameterConfig() = default;
    
    //! Erstellt ein Objekt mit is_fitted=true und dem gegebenen Zielparameter.
    explicit ModelParameterConfig(std::string model_parameter,
                            std::string target_parameter) :
            model_parameter(model_parameter),
            is_fitted(true),
            target_parameter(target_parameter)
    {
    }
    
    //! Erstellt ein Objekt mit is_fitted=false und dem gegebenen Fixwert.
    explicit ModelParameterConfig(std::string model_parameter,
                            double fixed_value) :
            model_parameter(model_parameter),
            is_fitted(false),
            fixed_value(fixed_value)
    {
    }
    
    //! Name des Modellparameters.
    std::string model_parameter;
    
    //! Wenn Wahr wird dieser Modellparameter gefittet.
    /*!
     * target_parameter enthält den Namen des zugehörigen Fitparameters.
     */
    bool is_fitted;
    
    //! Enthält den Namen des zugehörigen Fitparameters, falls gefittet wird.
    std::string target_parameter;
    
    //! Fester Wert des Modellparameters, falls nicht gefittet wird.
    double fixed_value;
};

typedef std::vector<ModelParameterConfig> ModelParameterConfigs;

#endif // MODELPARAMETERCONFIG_H