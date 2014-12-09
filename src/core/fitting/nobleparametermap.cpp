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


#include <algorithm>
#include <cassert>
#include <map>
#include <set>

#include <boost/foreach.hpp>

#include "fitparameterconfig.h"
#include "nobleparametermap.h"

bool NobleParameterMap::Parameter::operator==(
        const NobleParameterMap::Parameter& other) const
{
    if (fitted != other.fitted)
        return false;
    if (fitted)
        return target_index == other.target_index;
    return value == other.value;
}

class NobleParameterMap::PairIsSecondNegative
{
public:
    bool operator()(const std::pair<int, int>& p)
    {
        return p.second < 0;
    }
};

class NobleParameterMap::PairFirstGreater
{
public:
    bool operator()(const std::pair<int, double>& a, const std::pair<int, double>& b)
    {
        return a.first > b.first;
    }
};

class NobleParameterMap::PairSecondLess
{
public:
    bool operator()(const std::pair<int, int>& a, const std::pair<int, int>& b)
    {
        return a.second < b.second;
    }
};

NobleParameterMap::NobleParameterMap(
        std::shared_ptr<const CombinedModel> model,
        const FitParameterConfig& fit_parameter_config,
        const std::vector<ModelParameterConfigs>& model_parameter_configs) :
    current_map_(),
    original_map_(model_parameter_configs.size())
{
    std::vector<std::string> model_parameter_names_in_order =
            model->GetParameterNamesInOrder();
    
    std::set<std::string> model_parameter_names =
            model->GetParameterNames();
    
    std::map<std::string, unsigned> fit_parameter_indices =
            fit_parameter_config.GetNameToIndexMap();
    
    /* Maps (für jede Probe die am Fit teilnimmt) erzeugen, die Modellparameter-
     * Namen auf die entsprechenden Indizes in model_parameter_configs abbilden
     */
    std::vector<std::map<std::string, unsigned>> maps(model_parameter_configs.size());
    unsigned i = 0U;
    for (auto& map : maps)
    {
        unsigned j = 0U;
        for (const auto& current_parameter : model_parameter_configs[i])
        {
            if (!model_parameter_names.count(current_parameter.model_parameter))
                throw NobleParameterMapError("Model parameter " +
                                             current_parameter.target_parameter
                                             + " not found.");
            map[current_parameter.model_parameter] = j++;
        }
        ++i;
    }
    
    // Schleife über alle Proben.
    unsigned k = 0U;
    for (auto& final_map : original_map_)
    {
        // Schleife über alle vom Modell benötigten Parameter.
        for (const auto& parameter : model_parameter_names_in_order)
        {
            // zugehörigen Index in model_parameter_configs finden.
            auto it = maps[k].find(parameter);
            if (it == maps[k].cend())
                throw NobleParameterMapError("Could not find parameter " +
                                             parameter +
                                             "which is required by the model");
            unsigned index = it->second;
            const ModelParameterConfig& model_parameter = model_parameter_configs[k][index];
            
            final_map.push_back(Parameter());
            if ((final_map.back().fitted = model_parameter.is_fitted))
            {
                // Prüfen ob der gewählte Fitparameter existiert.
                auto jt = fit_parameter_indices.find(
                        model_parameter.target_parameter);
                if (jt == fit_parameter_indices.cend())
                    throw NobleParameterMapError("Fit parameter " +
                            model_parameter.target_parameter + " not found.");
                    
                final_map.back().target_index = jt->second;
            }
            else
                final_map.back().value = model_parameter.fixed_value;
        }
        ++k;
    }
    
    current_map_ = original_map_;
    
    ValidateParameterMap();
    DetermineIndicesOfFittedParameters();
}

void NobleParameterMap::FixParameters(std::vector<std::pair<int, double> > parameters)
{
    std::sort(parameters.begin(), parameters.end(), PairFirstGreater());

    current_map_ = original_map_;

    for (std::vector<std::pair<int, double> >::const_iterator it = parameters.begin();
         it != parameters.end();
         ++it)
        for (std::vector<std::vector<Parameter> >::iterator jt = current_map_.begin();
             jt != current_map_.end();
             ++jt)
            for (std::vector<Parameter>::iterator kt = jt->begin(); kt != jt->end(); ++kt)
                if (kt->fitted)
                {
                    if (kt->target_index == it->first)
                    {
                        kt->fitted = false;
                        kt->value = it->second;
                    }
                    else if (kt->target_index > it->first)
                        --kt->target_index;
                }

    DetermineIndicesOfFittedParameters();
}

void NobleParameterMap::ResetParameters()
{
    current_map_ = original_map_;
}

std::vector<Eigen::VectorXd> NobleParameterMap::MapParameterValues(const Eigen::VectorXd &fit_parameters) const
{
    assert(fit_parameters.size() == GetNumberOfFitParameters());

    std::vector<Eigen::VectorXd> parameters(current_map_.size());

    for (unsigned i = 0; i < current_map_.size(); ++i)
    {
        parameters[i].resize(current_map_[i].size());
        for (unsigned j = 0; j < current_map_[i].size(); ++j)
        {
            const Parameter& parameter = current_map_[i][j];
            parameters[i][j] = parameter.fitted ?
                               fit_parameters(parameter.target_index) :
                               parameter.value;
        }
    }

    return parameters;
}

const std::vector<int>& NobleParameterMap::GetIndicesOfFittedParameters(int sample_index) const
{
    return indices_of_fitted_parameters_[sample_index];
}

unsigned NobleParameterMap::GetNumberOfSamples() const
{
    assert(original_map_.size() == indices_of_fitted_parameters_.size());
    return original_map_.size();
}

unsigned NobleParameterMap::GetNumberOfFitParameters() const
{
    if (indices_of_fitted_parameters_.size() == 0)
        return 0;

    return indices_of_fitted_parameters_[0].size();
}

bool NobleParameterMap::operator==(const NobleParameterMap& other) const
{
    return current_map_ == other.current_map_ &&
           original_map_ == other.original_map_ &&
           indices_of_fitted_parameters_ == other.indices_of_fitted_parameters_;
}

//! \todo prüfen ob entfernt werden kann
void NobleParameterMap::ValidateParameterMap() const
{
    if (original_map_.size() > 0)
    {
        unsigned n = original_map_[0].size();

        for (unsigned i = 1; i < original_map_.size(); ++i)
            if (original_map_[i].size() != n)
                throw NobleParameterMapError("Parameter numbers mismatch.");
    }
}

//! \todo prüfen ob entfernt/ersetzt werden kann.
void NobleParameterMap::DetermineIndicesOfFittedParameters()
{
    int max = -1;

    BOOST_FOREACH (std::vector<Parameter>& i, current_map_)
        BOOST_FOREACH (Parameter& parameter, i)
            if (parameter.fitted && parameter.target_index > max)
                max = parameter.target_index;

    indices_of_fitted_parameters_ =
            std::vector<std::vector<int> >(current_map_.size(), std::vector<int>(max + 1, -1));

    int n = 0;
    BOOST_FOREACH (std::vector<Parameter>& i, current_map_)
    {
        std::vector<std::pair<int, int> > indices;

        int k = 0;
        BOOST_FOREACH (Parameter& j, i)
        {
            indices.push_back(std::make_pair(k, j.fitted ? j.target_index : -1));
            ++k;
        }

        indices = std::vector<std::pair<int, int> >(
                indices.begin(),
                std::remove_if(indices.begin(), indices.end(), PairIsSecondNegative()));

        std::sort(indices.begin(), indices.end(), PairSecondLess());

        std::vector<std::pair<int, int> >::const_iterator it = indices.begin();
        for (int l = 0; l < max + 1 && it != indices.end(); ++l)
        {
            if (l < it->second)
                continue;

            indices_of_fitted_parameters_[n][l] = it++->first;
        }

        ++n;
    }
}
