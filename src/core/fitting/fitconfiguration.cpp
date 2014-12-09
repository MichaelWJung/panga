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


#include "fitconfiguration.h"

FitConfiguration::FitConfiguration() :
    model(),
    fit_parameter_config(),
    model_parameter_configs(),
    n_monte_carlos(0UL),
    sample_numbers(),
    parameter_map_()
{
}

FitConfiguration::FitConfiguration(const FitConfiguration& other) :
    model(other.model->clone()),
    fit_parameter_config(other.fit_parameter_config),
    model_parameter_configs(other.model_parameter_configs),
    n_monte_carlos(other.n_monte_carlos),
    sample_numbers(other.sample_numbers),
    parameter_map_()
{
    if (other.parameter_map_)
        parameter_map_ =
                std::make_shared<NobleParameterMap>(*other.parameter_map_);
}

std::shared_ptr<NobleParameterMap> FitConfiguration::GetParameterMap() const
{
    if (!parameter_map_)
        CreateParameterMap();
    return parameter_map_;
}

void FitConfiguration::CreateParameterMap() const
{
    parameter_map_ = std::make_shared<NobleParameterMap>(
        model, fit_parameter_config, model_parameter_configs);
}

