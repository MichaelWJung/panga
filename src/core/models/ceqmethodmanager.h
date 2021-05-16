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


#ifndef CEQMETHODMANAGER_H
#define CEQMETHODMANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ceqmethodfactory.h"

//! Lädt Modelle aus dem plugins-Unterverzeichnis und erstellt sie bei Bedarf.
/*!
 * \todo Die gesamte Modellerstellung gehört refaktorisiert, zumindest diese
 * Klasse.
 * 
 * Singleton.
 */
class CEqMethodManager
{

public:

    //! Gibt den PluginManager zurück.
    static const CEqMethodManager& Get();
    
    //! Gibt die gewünschte ModelFactory zurück.
    /*!
      \param name Name des Modells.
      \return Zeiger auf die ModelFactory.
      */
    CEqMethodFactory* GetCEqMethodFactory(const std::string& name) const;

    //! Erzeugt eine Liste aller verfügbaren Modelle.
    std::vector<std::string> GetAvailableCEqMethods() const;
    
    class CEqMethodNotFoundError {};


private:

    //! Konstruiert das Objekt und lädt alle Modelle aus dem plugins-Unterverzeichnis.
    CEqMethodManager();
    CEqMethodManager(const CEqMethodManager&) = delete;
    CEqMethodManager& operator=(const CEqMethodManager&) = delete;
    
    typedef std::map<std::string, CEqMethodFactory*> MapType;
    //! Map mit Zeigern auf alle verfügbaren Modelle.
    MapType ceqmethod_factories_;

    static const std::vector<std::shared_ptr<CEqMethodFactory>> CEQMETHOD_FACTORIES;
};

#endif // CEQMETHODMANAGER_H
