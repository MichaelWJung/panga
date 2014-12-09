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


#ifndef FITCONFIGURATION_H
#define FITCONFIGURATION_H

#include <memory>

#include "fitparameterconfig.h"
#include "nobleparametermap.h"

#include "core/models/combinedmodelfactory.h"

//! Bündelt alle notwendigen Einstellungen zum Durchführen eines Fits.
/*!
 * Die Gaskonzentrationen sind jedoch nicht enthalten.
 */
class FitConfiguration
{
public:
    FitConfiguration();
    
    FitConfiguration(const FitConfiguration& other);
    
    std::shared_ptr<NobleParameterMap> GetParameterMap() const;
    void CreateParameterMap() const;
    
    std::shared_ptr<CombinedModel> model;
    FitParameterConfig fit_parameter_config;
    std::vector<ModelParameterConfigs> model_parameter_configs;
    unsigned long n_monte_carlos;
    std::vector<unsigned> sample_numbers;

    
private:
    mutable std::shared_ptr<NobleParameterMap> parameter_map_;
};

#endif // FITCONFIGURATION_H
