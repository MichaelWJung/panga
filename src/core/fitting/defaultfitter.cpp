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


#include "core/misc/defines.h"

#include "levenbergmarquardtfitter.h"
#include "noblefitfunction.h"
#include "montecarlocontroller.h"
#include "randomnumberbuffer.h"
#include "randomnumbergenerator.h"

#include "defaultfitter.h"

DefaultFitter::DefaultFitter(
        std::shared_ptr<FitResultsProcessor> results_processor) :
    concentrations_(),
    fit_configurations_(),
    results_processor_(results_processor)
{
}

void DefaultFitter::SetConcentrations(const RunData& concentrations)
{
    concentrations_ = concentrations;
}

void DefaultFitter::SetFitConfigurations(
        const std::vector<FitConfiguration>& fit_configurations)
{
    fit_configurations_ = fit_configurations;
}

void DefaultFitter::Fit()
{
    unsigned n_cpus = boost::thread::hardware_concurrency();
    if (n_cpus == 0) n_cpus = 1;
        
    unsigned n_samples = fit_configurations_.size();
    
    std::vector<std::shared_ptr<FitResults>> results(n_samples);

    std::vector<SampleConcentrations> concentrations(
            concentrations_.concentrations_begin(),
            concentrations_.concentrations_end());
    std::vector<std::string> sample_names = concentrations_.GetEnabledSampleNames();
    
    std::vector<std::vector<SampleConcentrations>> concentrations_used_by_fits(n_samples);
    std::vector<std::vector<std::string>> samples_used_by_fits(n_samples);

    boost::mutex mutex;
    boost::thread_group threads;
    unsigned counter = 0;
    for (unsigned l = 0; l < n_cpus; ++l)
        threads.create_thread(std::bind(&DefaultFitter::PerformFits,
                                        this,
                                        std::ref(mutex),
                                        std::ref(counter),
                                        std::cref(concentrations),
                                        std::cref(sample_names),
                                        std::ref(samples_used_by_fits),
                                        std::ref(concentrations_used_by_fits),
                                        std::ref(results)));
        threads.join_all();

    for (unsigned i = 0; i < n_samples; ++i)
        results_processor_->ProcessResult(results[i],
                                          samples_used_by_fits[i],
                                          fit_configurations_[i].
                                              fit_parameter_config.names(),
                                          concentrations);
                                  
    unsigned long n_fits = 0UL;
    std::vector<unsigned long> n_monte_carlos(n_samples);

    for (unsigned i = 0; i < n_samples; ++i)
        n_fits += n_monte_carlos[i] = fit_configurations_[i].n_monte_carlos;

    if (n_fits)
    {
        MonteCarloController controller(n_monte_carlos);
        RandomNumberGenerator random_number_generator(
                static_cast<unsigned>(std::time(0)));

        boost::thread_group worker_threads;

        for (unsigned i = 0; i < n_cpus; ++i)
            worker_threads.create_thread(
                        std::bind(&DefaultFitter::PerformMonteCarloFits,
                                  this,
                                  std::ref(controller),
                                  std::ref(random_number_generator),
                                  std::ref(concentrations_used_by_fits)));
        try
        {
            try
            {
                while(true)
                {
                    std::pair<unsigned, std::shared_ptr<FitResults> >
                            monte_carlo_result = controller.GetNextResult();

                    results_processor_->ProcessMonteCarloResult(
                        std::dynamic_pointer_cast<FitResults>(
                                monte_carlo_result.second),
                        samples_used_by_fits[monte_carlo_result.first],
                        fit_configurations_[monte_carlo_result.first].
                                fit_parameter_config.names(),
                        concentrations);
                }
            }
            catch(MonteCarloController::NoResultsLeft)
            {
            }

            worker_threads.join_all();
        }
        catch(...)
        {
            worker_threads.interrupt_all();
            worker_threads.join_all();
            throw;
        }
    }
}

noble_align_function void DefaultFitter::PerformFits(
        boost::mutex& mutex,
        unsigned& counter,
        const std::vector<SampleConcentrations>& concentrations,
        const std::vector<std::string>& sample_names,
        std::vector<std::vector<std::string>>& samples_used_by_fits,
        std::vector<std::vector<SampleConcentrations>>& concentrations_used_by_fits,
        std::vector<std::shared_ptr<FitResults>>& results) const
{
    unsigned n_samples = fit_configurations_.size();
    while(true)
    {
        unsigned i;
        {
            boost::lock_guard<boost::mutex> lock(mutex);
            i = counter++;
            if (counter > n_samples) return;
        }

        const FitConfiguration& config = fit_configurations_[i];
        std::vector<SampleConcentrations>& concentrations_used_by_fit =
                concentrations_used_by_fits[i];
        std::vector<std::string>& samples_used_by_fit =
                samples_used_by_fits[i];

        for (auto j : config.sample_numbers)
        {
            concentrations_used_by_fit.push_back(concentrations[j]);
            samples_used_by_fit.push_back(sample_names[j]);
        }

        std::shared_ptr<NobleFitFunction> function(
                std::make_shared<NobleFitFunction>(
                    config.model,
                    *config.GetParameterMap(),
                    concentrations_used_by_fit));

        LevenbergMarquardtFitter fitter(function);

        std::shared_ptr<FitResults> results_of_single_fit(
                std::dynamic_pointer_cast<FitResults>(
                    fitter.fit(std::make_shared<FitParameterConfig>(
                        config.fit_parameter_config))));

        assert(results_of_single_fit);

        results[i] = results_of_single_fit;
    }
}

noble_align_function void DefaultFitter::PerformMonteCarloFits(
        MonteCarloController& controller,
        RandomNumberGenerator& generator,
        const std::vector<std::vector<SampleConcentrations>>& concentrations
        ) const
{
    RandomNumberBuffer rnd(generator);

    try
    {
        unsigned i = 0;

        //So lange Jobs abarbeiten bis MonteCarloController::NoJobsLeft geworfen wird.
        while(true)
        {
            std::shared_ptr<boost::promise<
                    std::shared_ptr<FitResults> > > promise(
                        controller.GetNewJob(i));

            std::vector<std::map<GasType, Data> > varied_concentrations(concentrations[i]);

            for (unsigned j = 0; j < varied_concentrations.size(); ++j)
                for (auto it = varied_concentrations[j].begin();
                     it != varied_concentrations[j].end();
                     ++it)
                {
                    it->second.value += rnd() * it->second.error;
                }
                
            const FitConfiguration& config = fit_configurations_[i];

            std::shared_ptr<NobleFitFunction> function(
                        std::make_shared<NobleFitFunction>(
                            config.model,
                            *config.GetParameterMap(),
                            varied_concentrations));

            LevenbergMarquardtFitter fitter(function);

            promise->set_value(fitter.fit(std::make_shared<FitParameterConfig>(
                            config.fit_parameter_config)));

            boost::this_thread::interruption_point();
        }
    }
    catch(MonteCarloController::NoJobsLeft)
    {
    }
}

