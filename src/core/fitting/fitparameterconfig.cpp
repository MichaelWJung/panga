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
#include <stdexcept>

#include "fitparameterconfig.h"

void FitParameterConfig::AddParameter(const FitParameter& parameter)
{
    if (std::find(names_.cbegin(), names_.cend(), parameter.name) !=
            names_.end())
        throw std::invalid_argument("Parameter " + parameter.name +
                                    " already exists.");
        
    names_.push_back(parameter.name);

	//Workaround weil conservativeResize nicht richtig funktioniert.
	unsigned size = initials_.size();
	Eigen::VectorXd helper(size);
	helper.swap(initials_);
	initials_.resize(size + 1);
	initials_.topRows(size).swap(helper);
	initials_(size) = parameter.initial_value;
}

size_t FitParameterConfig::size() const
{
    return names_.size();
}

const std::vector< std::string >& FitParameterConfig::names() const
{
    return names_;
}

const Eigen::VectorXd& FitParameterConfig::initials() const
{
    return initials_;
}

void FitParameterConfig::ChangeParameterInitial(unsigned int index,
                                                double new_value)
{
    initials_[index] = new_value;
}

void FitParameterConfig::ChangeParameterInitial(const std::string& name, double new_value)
{
    auto it = std::find(names_.begin(), names_.end(), name);
    if (it == names_.end())
        throw std::invalid_argument("Parameter not found.");
    unsigned index = it - names_.begin();
    ChangeParameterInitial(index, new_value);
}

void FitParameterConfig::RemoveParameter(unsigned int index)
{
    names_.erase(names_.begin() + index);
    
    const unsigned n = initials_.size() - index - 1;
    const unsigned new_size = initials_.size() - 1;
    initials_.segment(index, n) = initials_.tail(n);
    initials_.conservativeResize(new_size);
}

std::map< std::string, unsigned > FitParameterConfig::GetNameToIndexMap() const
{
    std::map<std::string, unsigned> map;
    unsigned l = 0U;
    for (const auto& name : names())
        map[name] = l++;
    
    return map;
}
