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


#ifndef FITPARAMETERCONFIG_H
#define FITPARAMETERCONFIG_H

#include <Eigen/Core>

#include <map>
#include <string>
#include <vector>

#include "fitparameter.h"

//! Wird zum Speichern von Parameter-Initialwerten und -Grenzen verwendet.
class FitParameterConfig
{
public:
    //! Fügt einen Parameter hinzu.
    void AddParameter(const FitParameter& parameter);
    
    //! Gibt die Zahl der Parameter zurück.
    size_t size() const;
    
    //! Gibt die Namen der Parameter zurück.
    const std::vector<std::string>& names() const;
    
    //! Gibt den Vektor der Anfangswerte für die Parameter zurück.
    const Eigen::VectorXd& initials() const;
    
    //! Ändert einen einzelnen Parameter-Initialwert.
    void ChangeParameterInitial(unsigned index, double new_value);
    
    //! Ändert einen einzelnen Parameter-Initialwert.
    void ChangeParameterInitial(const std::string& name, double new_value);
    
    //! Entfernt den Parameter mit dem übergebenen Index.
    void RemoveParameter(unsigned index);
    
    //! Erzeugt eine Map, die Fitparameter-Namen auf ihren Index abbildet.
    std::map<std::string, unsigned> GetNameToIndexMap() const;
    
private:
    //! Namen der Parameter.
    std::vector<std::string> names_;
    
    //! Vektor der Anfangswerte für die Parameter.
    Eigen::VectorXd initials_;
};

#endif // FITPARAMETERCONFIG_H
