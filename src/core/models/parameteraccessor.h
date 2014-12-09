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


#ifndef PARAMETERACCESSOR_H
#define PARAMETERACCESSOR_H

#include <Eigen/Core>

//! Diese Klasse dient dem einfachen Zugriff auf den aktuellen Parametersatz.
/*!
  Dieser Klasse wird eine Referenz auf den aktuellen Parametervektor übergeben. Die verschiedenen
  Excess-Air-Modelle und Gleichgewichts-Berechnungs-Methoden müssen Kindklassen implementieren,
  die wiederum Zeiger auf die von ihnen benötigten Parameter bereitstellen. Die Standardimplementierung
  kann mit \ref generatehelperclasses.h erzeugt werden.
  */
class ParameterAccessor
{

public:

    ~ParameterAccessor() {}

    //! Hiermit kann der zugrunde liegende Parametervektor geändert werden.
    virtual void SetVectorReference(const Eigen::VectorXd& parameters) = 0;


protected:

    ParameterAccessor() {}


private:

    ParameterAccessor(const ParameterAccessor&);
    ParameterAccessor& operator=(const ParameterAccessor&);
};

#endif // PARAMETERACCESSOR_H
