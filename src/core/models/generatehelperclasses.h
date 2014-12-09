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


#include <limits>

#ifndef NOBLE_PARAMETER_COUNT
    #error You need to define NOBLE_PARAMETER_COUNT in order to use generatequilibriumhelperclasses.h.
#endif

#ifndef NOBLE_PARAMETER_NAMES
    #error You need to define NOBLE_PARAMETER_NAMES in order to use generatequilibriumhelperclasses.h.
#endif

#ifndef NOBLE_CLASS_PREFIX
    #error You need to define NOBLE_CLASS_PREFIX in order to use generatequilibriumhelperclasses.h.
#endif

#ifndef NOBLE_IS_CEQ_CALCULATION_METHOD
    #ifndef NOBLE_IS_EXCESS_AIR_MODEL
        #error You need to define either NOBLE_IS_CEQ_CALCULATION_METHOD or NOBLE_IS_EXCESS_AIR_MODEL.
    #endif //NOBLE_IS_EXCESS_AIR_MODEL
#else //NOBLE_IS_CEQ_CALCULATION_METHOD
    #ifdef NOBLE_IS_EXCESS_AIR_MODEL
        #error You need to define either NOBLE_IS_CEQ_CALCULATION_METHOD or NOBLE_IS_EXCESS_AIR_MODEL.
    #endif //NOBLE_IS_EXCESS_AIR_MODEL
    #undef NOBLE_IS_CEQ_CALCULATION_METHOD
#endif //NOBLE_IS_CEQ_CALCULATION_METHOD

#include "parameteraccessorgenerator.h"
#include "derivativecollectorgenerator.h"

#undef NOBLE_IS_EXCESS_AIR_MODEL
#undef NOBLE_PARAMETER_NAMES

#define NOBLE_HELPER_CLASSES_INCLUDED