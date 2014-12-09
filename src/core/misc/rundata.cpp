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
#include <functional>
#include <stdexcept>

#include "rundata.h"
#include "templates.h"

const SampleConcentrations&
RunData::ReturnConcentrations::operator()(const RunData::Tuple& in) const
{
    return in.concentrations;
}

bool RunData::ReturnEnabled::operator()(const RunData::Tuple& in) const
{
    return in.enabled;
}

RunData::RunData() : data_() {}

RunData::RunData(const RunData& other) :
    QObject(),
    data_(other.data_)
{
}

RunData& RunData::operator=(const RunData& other)
{
    if (&other != this)
        data_ = other.data_;
    return *this;
}

void RunData::Add(Sample&& sample)
{
    data_.emplace_back(std::move(sample.first), std::move(sample.second));
}

size_t RunData::EnabledSize() const
{
    return std::count_if(data_.cbegin(), data_.cend(),
                         [](const Tuple& t)
                         {
                             return t.enabled;
                         });
}

size_t RunData::TotalSize() const
{
    return data_.size();
}

RunData::const_concentrations_iterator RunData::concentrations_begin() const
{
    return boost::make_transform_iterator(
            boost::make_filter_iterator(ReturnEnabled(),
                                        data_.cbegin(),
                                        data_.cend()),
            ReturnConcentrations());
}

RunData::const_concentrations_iterator RunData::concentrations_end() const
{
    return boost::make_transform_iterator(
            boost::make_filter_iterator(ReturnEnabled(),
                                        data_.cend(),
                                        data_.cend()),
            ReturnConcentrations());
}

std::vector<std::string> RunData::GetEnabledSampleNames() const
{
    std::vector<std::string> names;
    
    for (auto it = data_.begin(); it != data_.end(); ++it)
        if (it->enabled)
            names.push_back(it->name);
    
    return names;
}

std::vector< std::string > RunData::GetAllSampleNames() const
{
    std::vector<std::string> names;
    
    for (auto it = data_.begin(); it != data_.end(); ++it)
        names.push_back(it->name);
    
    return names;
}

const std::string& RunData::GetSampleName(unsigned index) const
{
    return data_.at(index).name;
}

const SampleConcentrations& RunData::GetSampleConcentrations(
        unsigned index) const
{
    return data_.at(index).concentrations;
}

void RunData::RemoveGas(GasType gas)
{
    for (auto& sample : data_)
        sample.concentrations.erase(gas);
}

void RunData::DisableSample(unsigned int index)
{
    bool changed = data_.at(index).enabled;
    data_.at(index).enabled = false;
    if (changed)
        emit SampleDisabled(index);
}

void RunData::EnableSample(unsigned int index)
{
    bool changed = !data_.at(index).enabled;
    data_.at(index).enabled = true;
    if (changed)
        emit SampleEnabled(index);
}

bool RunData::IsEnabled(unsigned index) const
{
    return data_.at(index).enabled;
}

void RunData::EnableAllSamples()
{
    for (unsigned i = 0; i < data_.size(); ++i)
        EnableSample(i);
}

void RunData::DisableAllSamples()
{
    for (unsigned i = 0; i < data_.size(); ++i)
        DisableSample(i);
}

void RunData::clear()
{
    data_.clear();
}
