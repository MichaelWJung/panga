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


#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <map>
#include <memory>

#include "data.h"
#include "gas.h"

typedef std::pair<GasType, Data> GasConcentration;
typedef std::map<GasType, Data> SampleConcentrations;
typedef std::pair<std::string, SampleConcentrations> Sample;

#endif //TYPEDEFS_H