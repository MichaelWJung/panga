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


#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "core/misc/parameter_description.h"
#include "core/models/parametermanager.h"

#include "excessairmodel.h"

//! Muss von Plugins zum Erzeugen von Modellen bereitgestellt werden.
class ModelFactory
{

public:

    virtual ~ModelFactory() {}

    //! Erzeugt das Modell.
    virtual std::shared_ptr<ExcessAirModel> CreateModel(
        std::shared_ptr<ParameterManager> manager
        ) const = 0;

    virtual std::string GetModelName() const = 0;
};

#endif // MODELFACTORY_H
