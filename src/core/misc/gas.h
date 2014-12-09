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


#ifndef GAS_H
#define GAS_H

#include <string>
#include <boost/algorithm/string.hpp>

//! Die im Programm verwendbaren Gase.
struct Gas
{
    //! Die verwendbaren Gase.
    /*!
      Warnung: Nicht leichtfertig verändern. Andere Programmteile müssen
      angepasst werden.
      */
    enum GasType
    {
        begin = 0,
        HE = 0,
        NE = 1,
        AR = 2,
        KR = 3,
        XE = 4,
        end = 5,
        HE3 = 5,
        end_including_HE3 = 6,
        INVALID = 255
    };

    static GasType StringToGasType(std::string string)
    {
        boost::to_lower(string);
        if (string == "he") return HE;
        if (string == "ne") return NE;
        if (string == "ar") return AR;
        if (string == "kr") return KR;
        if (string == "xe") return XE;

        return INVALID;
    }

    static std::string GasTypeToString(GasType gas)
    {
        std::string ret;

        switch (gas)
        {
        case HE:
            ret = "He";
            break;
        case NE:
            ret = "Ne";
            break;
        case AR:
            ret = "Ar";
            break;
        case KR:
            ret = "Kr";
            break;
        case XE:
            ret = "Xe";
            break;
        case HE3:
            ret = "³He";
            break;
        default:
            ret = "invalid";
            break;
        }

        return ret;
    }

};

typedef Gas::GasType GasType;

inline GasType& operator++(GasType& g)
{
    assert(g < Gas::end_including_HE3);
    g = static_cast<GasType>(g + 1);
    return g;
}

#endif // GAS_H
