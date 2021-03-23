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


#ifndef TESTCEQMETHOD_H
#define TESTCEQMETHOD_H

#include "core/models/ceqcalculationmethod.h"
#include "core/models/ceqmethodfactory.h"
#include "core/models/parametermanager.h"

#define NOBLE_PARAMETER_COUNT 3
#define NOBLE_PARAMETER_NAMES (p, S, T)
#define NOBLE_CLASS_PREFIX TestCEq
#define NOBLE_IS_CEQ_CALCULATION_METHOD
#include "core/models/generatehelperclasses.h"

class TestCEqMethod : public CEqCalculationMethod
{
public:
    TestCEqMethod(std::shared_ptr<ParameterManager> manager) :
        pi_(manager->RegisterParameter("p", "atm")),
        Si_(manager->RegisterParameter("S", "g/kg")),
        Ti_(manager->RegisterParameter("T", "°C"))
    {
    }

    std::shared_ptr<ParameterAccessor> GetParameterAccessor() const
    {
        return std::make_shared<TestCEqParameterAccessor>(pi_, Si_, Ti_);
    }

    std::shared_ptr<DerivativeCollector> GetDerivativeCollector() const
    {
        return std::make_shared<TestCEqDerivativeCollector>(pi_, Si_, Ti_);
    }
    double CalculateConcentration(
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const
    {
        std::shared_ptr<TestCEqParameterAccessor> x(
                    std::dynamic_pointer_cast<TestCEqParameterAccessor>(parameters));
        return (x->p() + x->S() + x->T());
    }

    void CalculateDerivatives(
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const
    {
        std::shared_ptr<TestCEqParameterAccessor> x(
                    std::dynamic_pointer_cast<TestCEqParameterAccessor>(parameters));
        std::shared_ptr<TestCEqDerivativeCollector> y(
                    std::dynamic_pointer_cast<TestCEqDerivativeCollector>(derivatives));

        const std::vector<std::pair<TestCEq::Parameter, double*> >& infos = y->GetDerivativeInformation();

        for (std::vector<std::pair<TestCEq::Parameter, double*> >::const_iterator it = infos.begin();
             it != infos.end();
             ++it)
        {
            switch (it->first)
            {
            case TestCEq::p:
                *it->second = 1.;
                break;

            case TestCEq::S:
                *it->second = 2.;
                break;

            case TestCEq::T:
                *it->second *= 42;
                break;

            case TestCEq::OTHER:
                *it->second *= 1e30;
                break;

            default:
                *it->second = 0;
                break;
            }
        }
    }

    std::string GetCEqMethodName() const
    {
        return "TestCEq";
    }

private:
    const unsigned pi_;
    const unsigned Si_;
    const unsigned Ti_;
};

class TestCEqFactory : public CEqMethodFactory
{
public:
    ~TestCEqFactory() {}

    std::shared_ptr<CEqCalculationMethod> CreateCEqMethod(
        std::shared_ptr<ParameterManager> manager
        ) const

    {
        return std::make_shared<TestCEqMethod>(manager);
    }

    std::string GetCEqMethodName() const
    {
        return "TestCEq";
    }
};

#endif // TESTCEQMETHOD_H
