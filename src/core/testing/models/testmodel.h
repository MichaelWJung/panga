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


#ifndef TESTMODEL_H
#define TESTMODEL_H

#include "core/models/excessairmodel.h"
#include "core/models/modelfactory.h"
#include "core/models/parametermanager.h"

#define NOBLE_PARAMETER_COUNT 4
#define NOBLE_PARAMETER_NAMES (a, b, c, T)
#define NOBLE_CLASS_PREFIX Test
#define NOBLE_IS_EXCESS_AIR_MODEL
#include "core/models/generatehelperclasses.h"

class TestModel : public ExcessAirModel
{
public:
    TestModel(std::shared_ptr<ParameterManager> manager) :
        ai_(manager->RegisterParameter("a", "baenk")),
        bi_(manager->RegisterParameter("b", "boenk")),
        ci_(manager->RegisterParameter("c", "buenk")),
        Ti_(manager->RegisterParameter("T", "°C"))
    {
    }

    std::shared_ptr<ParameterAccessor> GetParameterAccessor() const
    {
        return std::make_shared<TestParameterAccessor>(ai_, bi_, ci_, Ti_);
    }

    std::shared_ptr<DerivativeCollector> GetDerivativeCollector() const
    {
        return std::make_shared<TestDerivativeCollector>(ai_, bi_, ci_, Ti_);
    }

    double CalculateConcentration(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        GasType gas
        ) const
    {
        std::shared_ptr<TestParameterAccessor> x(
                    std::dynamic_pointer_cast<TestParameterAccessor>(parameters));
        return (x->a() + x->b() + x->c() + x->T()) * c_eq;
    }

    void CalculateDerivatives(
        double c_eq,
        std::shared_ptr<ParameterAccessor> parameters,
        std::shared_ptr<DerivativeCollector> derivatives,
        GasType gas
        ) const
    {
        std::shared_ptr<TestParameterAccessor> x(
                    std::dynamic_pointer_cast<TestParameterAccessor>(parameters));
        std::shared_ptr<TestDerivativeCollector> y(
                    std::dynamic_pointer_cast<TestDerivativeCollector>(derivatives));

        const std::vector<std::pair<Test::Parameter, double*> >& infos = y->GetDerivativeInformation();

        for (std::vector<std::pair<Test::Parameter, double*> >::const_iterator it = infos.begin();
             it != infos.end();
             ++it)
        {
            switch (it->first)
            {
            case Test::a:
                *it->second = 1.;
                break;

            case Test::b:
                *it->second = 2.;
                break;

            case Test::c:
                *it->second = 3.;
                break;

            case Test::T:
                *it->second *= 42;
                break;

            case Test::OTHER:
                *it->second *= 1e30;
                break;

            default:
                *it->second = 0;
                break;
            }
        }
    }

    std::string GetModelName() const
    {
        return "Test";
    }

private:
    const unsigned ai_;
    const unsigned bi_;
    const unsigned ci_;
    const unsigned Ti_;
};

class TestFactory : public ModelFactory
{
public:
    ~TestFactory() {}

    std::shared_ptr<ExcessAirModel> CreateModel(
        std::shared_ptr<ParameterManager> manager
        ) const

    {
        return std::make_shared<TestModel>(manager);
    }

    std::string GetModelName() const
    {
        return "Test";
    }
};

#endif // TESTMODEL_H
