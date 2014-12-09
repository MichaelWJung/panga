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


#ifndef FITPARAMETER_H
#define FITPARAMETER_H

#include <limits>
#include <string>
#include <boost/iterator/iterator_concepts.hpp>

struct FitParameter
{
    FitParameter(std::string name, double initial_value) :
            name(name),
            initial_value(initial_value)
    {
    }
    
    std::string name;
    double initial_value;
};

#endif // FITPARAMETER_H