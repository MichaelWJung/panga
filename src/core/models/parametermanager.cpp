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


#include <stdexcept>

#include "parametermanager.h"

ParameterManager::ParameterManager()
{
}

unsigned ParameterManager::RegisterParameter(const std::string &name,
                                             const std::string &unit,
                                             double default_value,
                                             double lowest_normal_value,
                                             double highest_normal_value,
                                             double highest_normal_error)
{
    auto par = parameters_.insert(
            std::make_pair(name, ModelParameter(name,
                                                unit,
                                                default_value,
                                                lowest_normal_value,
                                                highest_normal_value,
                                                highest_normal_error,
                                                parameters_.size()
                                                ))).first;

    if (par->second.unit != unit)
        throw std::runtime_error(
                "Error: the same parameter name (" + name + ") + has been "
                "claimed multiple times with different units(" +
                par->second.unit + ", " + unit + "). You need to reimplement "
                "your model and either change the parameter name or use the "
                "same unit.");
    if (par->second.lowest_normal_value < lowest_normal_value)
        par->second.lowest_normal_value = lowest_normal_value;
    if (par->second.highest_normal_value > highest_normal_value)
        par->second.highest_normal_value = highest_normal_value;
    if (par->second.highest_normal_error > highest_normal_error)
        par->second.highest_normal_error = highest_normal_error;

    return par->second.index;
}

unsigned ParameterManager::GetParameterIndex(const std::string &name) const
{
    auto it = parameters_.find(name);

    if (it == parameters_.end())
        throw ParameterNotFound();

    return it->second.index;
}

std::vector<ModelParameter> ParameterManager::GetParameters() const
{
    std::vector<ModelParameter> list;

    for (auto it = parameters_.cbegin(); it != parameters_.cend(); ++it)
        list.push_back(it->second);

    return list;
}

std::vector<ModelParameter> ParameterManager::GetParametersInOrder() const
{
    std::vector<ModelParameter> vec(parameters_.size());

    for (auto it = parameters_.cbegin(); it != parameters_.cend(); ++it)
        vec[it->second.index] = it->second;

    return vec;
}
