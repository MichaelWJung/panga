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

template <class Vec, class Pred>
unsigned TransformSequenceIndexToSkipElements(Vec vec,
                                              unsigned index,
                                              Pred pred)
{
    auto it = std::find_if(vec.cbegin(), vec.cend(), pred);
    if (it == vec.cend())
        throw std::out_of_range("Index out of range.");
    
    for (unsigned i = 0; i < index; ++i)
        while (true)
        {
            if (++it == vec.cend())
                throw std::out_of_range("Index out of range.");
            if (pred(*it)) break;
        }
    return it - vec.begin();
}