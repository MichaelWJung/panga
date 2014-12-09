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


#include <QApplication>
#include <QDir>
#include <QPluginLoader>

#include <boost/bind.hpp>

#include <algorithm>
#include <stdexcept>

#include "cemodelfactory.h"
#include "grmodelfactory.h"
#include "odmodelfactory.h"
#include "pdmodelfactory.h"
#include "prmodelfactory.h"
#include "uamodelfactory.h"

#include "modelmanager.h"

const std::vector<std::shared_ptr<ModelFactory>> ModelManager::MODEL_FACTORIES =
    {std::make_shared<CeModelFactory>(),
     std::make_shared<GrModelFactory>(),
     std::make_shared<OdModelFactory>(),
     std::make_shared<PdModelFactory>(),
     std::make_shared<PrModelFactory>(),
     std::make_shared<UaModelFactory>()};

const ModelManager& ModelManager::Get()
{
    static ModelManager manager;
    return manager;
}

ModelManager::ModelManager()
{
    for (auto factory : MODEL_FACTORIES)
        model_factories_[factory->GetModelName()] = factory.get();
}

ModelFactory* ModelManager::GetModelFactory(const std::string &name) const
{
    MapType::const_iterator it;

    it = model_factories_.find(name);

    if (it == model_factories_.end())
        throw ModelNotFoundError();

    return it->second;
}

std::vector<std::string> ModelManager::GetAvailableModels() const
{
    std::vector<std::string> ret;

    std::transform(model_factories_.begin(), model_factories_.end(),
                   std::back_inserter(ret),
                   boost::bind(&MapType::value_type::first, _1));

    return ret;
}
