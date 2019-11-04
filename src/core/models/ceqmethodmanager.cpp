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

#include "weissmethodfactory.h"
#include "jenkinsmethodfactory.h"

// #include "cemodelfactory.h"
// #include "grmodelfactory.h"
// #include "odmodelfactory.h"
// #include "pdmodelfactory.h"
// #include "prmodelfactory.h"
// #include "uamodelfactory.h"

#include "ceqmethodmanager.h"

const std::vector<std::shared_ptr<CEqMethodFactory>> CEqMethodManager::CEQMETHOD_FACTORIES =
    {// std::make_shared<CeModelFactory>(),
    //  std::make_shared<GrModelFactory>(),
    //  std::make_shared<OdModelFactory>(),
    //  std::make_shared<PdModelFactory>(),
    //  std::make_shared<PrModelFactory>(),
     std::make_shared<WeissMethodFactory>(),
     std::make_shared<JenkinsMethodFactory>()
     };

const CEqMethodManager& CEqMethodManager::Get()
{
    static CEqMethodManager manager;  //WIP: Wieso kein Konstuktor?
    return manager;
}

CEqMethodManager::CEqMethodManager()
{
    for (auto factory : CEQMETHOD_FACTORIES)
        ceqmethod_factories_[factory->GetCEqMethodName()] = factory.get(); //WIP: Woher kommt get?
}

CEqMethodFactory* CEqMethodManager::GetCEqMethodFactory(const std::string &name) const
{
    MapType::const_iterator it;

    it = ceqmethod_factories_.find(name);

    if (it == ceqmethod_factories_.end())
        throw CEqMethodNotFoundError();

    return it->second;
}

std::vector<std::string> CEqMethodManager::GetAvailableCEqMethods() const
{
    std::vector<std::string> ret;

    std::transform(ceqmethod_factories_.begin(), ceqmethod_factories_.end(),
                   std::back_inserter(ret),
                   boost::bind(&MapType::value_type::first, _1));

    return ret;
}
