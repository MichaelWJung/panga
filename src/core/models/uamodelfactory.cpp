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


#include "uamodel.h"

#include "uamodelfactory.h"

UaModelFactory::~UaModelFactory()
{
}

std::shared_ptr<ExcessAirModel> UaModelFactory::CreateModel(
    std::shared_ptr<ParameterManager> manager
    ) const
{
    return std::make_shared<UaModel>(manager);
}

std::string UaModelFactory::GetModelName() const
{
    return UaModel::NAME;
}

