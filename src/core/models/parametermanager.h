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


#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "modelparameter.h"

//! Klasse zur Verwaltung der Fitparameter.
/*!
  Die verschiedenen Modelle zur Berechnung der Gleichgewichtskonzentrationen von Gasen im Wasser sowie
  die Excess-Air-Modelle registrieren hier die von ihnen benötigten Parameter. Diese Klasse erstellt
  eine Liste aller verwendeten Parameter und berücksichtigt dabei auch, dass verschiedene Modelle die
  gleichen Parameter benötigen können.
  */
class ParameterManager
{

public:

    ParameterManager();

    //! Registriert einen neuen Parameter.
    /*!
      Wenn bereits ein Parameter mit gleichem Namen und gleicher Einheit registriert wurde wird dessen
        Index zurückgegeben.
      \param name Name des neuen Parameters.
      \param unit Einheit des Parameters.
      \param default_value Ein Default-Wert für diesen Parameter. Wird ignoriert
        falls der Parameter bereits vorhanden ist.
      \param lowest_normal_value Niedrigster normaler Wert, für alles niedrigere
        wird Monte Carlo empfohlen.
      \param highest_normal_value Höchster normaler Wert, für alles höhere
        wird Monte Carlo empfohlen.
      \param highest_normal_error Höchster normaler Fehler, für alles höhere
        wird Monte Carlo empfohlen.
      \return Index unter dem der Parameter im Parametervektor des Fits zu finden sein wird.
      \throw std::runtime_error Wird geworfen wenn versucht wird einen Parameter mit dem gleichen Namen
        wie ein bereits vorhandener, aber mit unterschiedlicher Einheit, zu registrieren.
      */
    unsigned RegisterParameter(
            const std::string& name,
            const std::string& unit,
            double default_value = 0,
            double lowest_normal_value =
                -std::numeric_limits<double>::infinity(),
            double highest_normal_value =
                std::numeric_limits<double>::infinity(),
            double highest_normal_error =
                std::numeric_limits<double>::infinity());

    //! Gibt den Index eines registrierten Parameters zurück.
    /*!
      Wirft ParameterNotFound, falls kein Parameter dieses Namens gefunden wurde.
      */
    unsigned GetParameterIndex(const std::string& name) const;

    //! Gibt eine Liste aller registrierten Parameter zurück.
    std::vector<ModelParameter> GetParameters() const;

    //! Gibt eine Liste aller registrierten Parameter, nach ihren Indizes geordnet, zurück.
    std::vector<ModelParameter> GetParametersInOrder() const;

    //! Wird geworfen, falls der gesuchte Parameter nicht gefunden werden kann.
    class ParameterNotFound {};


private:

    //! Liste aller registrierten Parameter
    /*!
      Unter dem Namen des verwendeten Parameters werden dessen Index, Einheit
      und Default-Wert gespeichert.
      */
    std::map<std::string, ModelParameter> parameters_;
};

#endif // PARAMETERMANAGER_H
