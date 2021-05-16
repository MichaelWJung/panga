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


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <memory>
#include <vector>

#include "core/fitting/fitparameterconfig.h"
#include "core/fitting/nobleparametermap.h"

#include "core/models/combinedmodel.h"
#include "core/models/combinedmodelfactory.h"

#include "testing/models/testmodel.h"
#include "testing/models/testceqmethod.h"

BOOST_AUTO_TEST_SUITE(NobleParameterMap_tests)

BOOST_AUTO_TEST_CASE(Constructor)
{
    CombinedModelFactory factory(new TestFactory(), new TestCEqFactory());
    std::shared_ptr<CombinedModel> model(factory.CreateModel());
    std::vector<std::string> parameters(model->GetParameterNamesInOrder());
    std::map<std::string, unsigned> name_map;
    
    unsigned i = 0U;
    for (const auto& p : parameters)
        name_map[p] = i++;
    
    FitParameterConfig config;
    config.AddParameter(FitParameter("B", 4));
    config.AddParameter(FitParameter("A", 1));
    
    std::vector<std::vector<ModelParameterConfig>> model_parameters(1); // 1 Probe
    model_parameters[0].push_back(ModelParameterConfig("b", 42));
    model_parameters[0].push_back(ModelParameterConfig("c", 23));
    model_parameters[0].push_back(ModelParameterConfig("a", "A"));
    model_parameters[0].push_back(ModelParameterConfig("S", 0));
    model_parameters[0].push_back(ModelParameterConfig("p", 1));
    
    // Falsche Zahl von Parametern
    BOOST_CHECK_THROW(NobleParameterMap(model, config, model_parameters),
                      NobleParameterMap::NobleParameterMapError);
    
    model_parameters[0].push_back(ModelParameterConfig("T", "X"));
    
    // Unbekannter Fitparameter
    BOOST_CHECK_THROW(NobleParameterMap(model, config, model_parameters),
                      NobleParameterMap::NobleParameterMapError);
    
    model_parameters[0].pop_back();
    model_parameters[0].push_back(ModelParameterConfig("X", "B"));
    
    // Unbekannter Modellparameter
    BOOST_CHECK_THROW(NobleParameterMap(model, config, model_parameters),
                      NobleParameterMap::NobleParameterMapError);
    
    model_parameters[0].pop_back();
    model_parameters[0].push_back(ModelParameterConfig("T", "B"));
    
    std::shared_ptr<NobleParameterMap> map;
    BOOST_REQUIRE_NO_THROW(map = std::make_shared<NobleParameterMap>(
            model,
            config,
            model_parameters));
    
    Eigen::VectorXd fit_parameters(2);
    fit_parameters << 4, 1;
    
    std::vector<Eigen::VectorXd> parameter_values;
    BOOST_REQUIRE_NO_THROW(parameter_values =
            map->MapParameterValues(fit_parameters));
            
    BOOST_REQUIRE_EQUAL(parameter_values.size(), 1);
    BOOST_CHECK_CLOSE(parameter_values[0][name_map["a"]],  1, 1e-10);
    BOOST_CHECK_CLOSE(parameter_values[0][name_map["b"]], 42, 1e-10);
    BOOST_CHECK_CLOSE(parameter_values[0][name_map["c"]], 23, 1e-10);
    BOOST_CHECK_CLOSE(parameter_values[0][name_map["T"]],  4, 1e-10);
}

BOOST_AUTO_TEST_SUITE_END()