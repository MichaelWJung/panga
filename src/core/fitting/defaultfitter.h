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


#ifndef DEFAULTFITTER_H
#define DEFAULTFITTER_H

#include <boost/random.hpp>
#include <memory>

#include "core/misc/rundata.h"
#include "core/fitting/fitconfiguration.h"

#include "fitresultsprocessor.h"

class MonteCarloController;
class RandomNumberGenerator;
namespace boost { class mutex; }

class DefaultFitter
{
public:
    DefaultFitter(std::shared_ptr<FitResultsProcessor> results_processor);
    
    void SetConcentrations(const RunData& concentrations);
    virtual void SetFitConfigurations(
            const std::vector<FitConfiguration>& fit_configurations);
    
    virtual void Fit();
    
private:
    void PerformFits(
            boost::mutex& mutex,
            unsigned& counter,
            const std::vector<SampleConcentrations>& concentrations,
            const std::vector<std::string>& sample_names,
            std::vector<std::vector<std::string>>& samples_used_by_fits,
            std::vector<std::vector<SampleConcentrations>>& concentrations_used_by_fits,
            std::vector<std::shared_ptr<FitResults>>& results) const;

    void PerformMonteCarloFits(
            MonteCarloController& controller,
            RandomNumberGenerator& generator,
            const std::vector<std::vector<SampleConcentrations>>& concentrations
            ) const;
        
    RunData concentrations_;
    std::vector<FitConfiguration> fit_configurations_;
    
    std::shared_ptr<FitResultsProcessor> results_processor_;
};

#endif // DEFAULTFITTER_H
