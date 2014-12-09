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


#ifndef MODELPARAMETER_H
#define MODELPARAMETER_H

#include <QString>

#include <limits>
#include <string>

struct ModelParameter
{
    ModelParameter() :
        name(),
        unit(),
        default_value(0.),
        lowest_normal_value(-std::numeric_limits<double>::infinity()),
        highest_normal_value(std::numeric_limits<double>::infinity()),
        highest_normal_error(std::numeric_limits<double>::infinity()),
        index(0)
    {
    }
    
    ModelParameter(const char* name) :
        name(name),
        unit(),
        default_value(0.),
        lowest_normal_value(-std::numeric_limits<double>::infinity()),
        highest_normal_value(std::numeric_limits<double>::infinity()),
        highest_normal_error(std::numeric_limits<double>::infinity()),
        index(0)
    {
    }
    
    ModelParameter(const std::string& name,
                   const std::string& unit,
                   double default_value,
                   unsigned index) :
        name(name),
        unit(unit),
        default_value(default_value),
        lowest_normal_value(-std::numeric_limits<double>::infinity()),
        highest_normal_value(std::numeric_limits<double>::infinity()),
        highest_normal_error(std::numeric_limits<double>::infinity()),
        index(index)
    {
    }
    
    ModelParameter(const std::string& name,
                   const std::string& unit,
                   double default_value,
                   double lowest_normal_value,
                   double highest_normal_value,
                   double highest_normal_error,
                   unsigned index) :
        name(name),
        unit(unit),
        default_value(default_value),
        lowest_normal_value(lowest_normal_value),
        highest_normal_value(highest_normal_value),
        highest_normal_error(highest_normal_error),
        index(index)
    {
    }
    
    std::string name;
    std::string unit;
    double default_value;
    double lowest_normal_value;
    double highest_normal_value;
    double highest_normal_error;
    unsigned index;
    
    QString GetName() const
    {
        return QString::fromStdString(name);
    }
    
    QString GetNameWithUnit() const
    {
        if (unit == "1" || unit.empty())
            return QString::fromStdString(name);
        return QString::fromStdString(name + " [" + unit + "]");
    }
    
    QString GetSuffixedNameWithUnit(const std::string& suffix) const
    {
        if (unit == "1" || unit.empty())
            return QString::fromStdString(name + suffix);
        return QString::fromStdString(name + suffix + " [" + unit + "]");
    }

    bool operator<(const ModelParameter& other) const
    {
        return index < other.index;
    }

    bool operator==(const ModelParameter& other) const
    {
        return name == other.name &&
               unit == other.unit &&
               default_value == other.default_value &&
               index == other.index;
    }
};

#endif // MODELPARAMETER_H