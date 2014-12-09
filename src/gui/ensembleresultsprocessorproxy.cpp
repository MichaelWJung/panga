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


#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <functional>
#include <stdexcept>

#include "ensembleresultsprocessorproxy.h"

using namespace std::placeholders;

EnsembleResultsProcessorProxy::EnsembleResultsProcessorProxy(
        const std::vector<std::string>& parameter_names,
        std::shared_ptr<FitResultsProcessor> processor) :
    parameter_names_(parameter_names),
    processor_(processor)
{
    if (!processor)
        throw std::invalid_argument(
                "Pointer to a FitResultsProcessor is invalid.");
}

void EnsembleResultsProcessorProxy::ProcessResult(
        std::shared_ptr<FitResults> results,
        const std::vector<std::string>& sample_names,
        const std::vector<std::string>& fit_parameter_names,
        const std::vector<SampleConcentrations>& concentrations)
{
    ConvertAndProcess(results,
                      sample_names,
                      fit_parameter_names,
                      concentrations,
                      std::bind(&FitResultsProcessor::ProcessResult,
                                processor_, _1, _2, _3, _4));
}

void EnsembleResultsProcessorProxy::ProcessMonteCarloResult(
        std::shared_ptr<FitResults> results,
        const std::vector<std::string>& sample_names,
        const std::vector<std::string>& fit_parameter_names,
        const std::vector<SampleConcentrations>& concentrations)
{
    ConvertAndProcess(results,
                      sample_names,
                      fit_parameter_names,
                      concentrations,
                      std::bind(&FitResultsProcessor::ProcessMonteCarloResult,
                                processor_, _1, _2, _3, _4));
}

template<typename F>
void EnsembleResultsProcessorProxy::ConvertAndProcess(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& fit_parameter_names,
            const std::vector<SampleConcentrations>& concentrations,
            F function)
{
    unsigned n_samples = sample_names.size();
    unsigned n_parameters = parameter_names_.size();
    unsigned total_number_of_parameters = results->best_estimate.size();
    unsigned n_shared_parameters =
            (n_parameters * n_samples - total_number_of_parameters) /
            (n_samples - 1);
    unsigned residuals_counter = 0;
    for (unsigned i = 0; i < n_samples; ++i)
    {
        std::vector<unsigned> parameter_indices =
                DetermineParameterIndices(n_shared_parameters,
                                          n_parameters,
                                          i,
                                          fit_parameter_names);
        std::shared_ptr<FitResults> individual_results(
                std::make_shared<FitResults>());
        individual_results->chi_square = results->chi_square;
        individual_results->best_estimate =
                SelectBestEstimates(results->best_estimate, parameter_indices);
        individual_results->deviations =
                SelectBestEstimates(results->deviations, parameter_indices);
        individual_results->covariance_matrix =  SelectCovariances(
                results->covariance_matrix, parameter_indices);
        individual_results->n_iterations = results->n_iterations;
        individual_results->degrees_of_freedom = results->degrees_of_freedom;
        individual_results->exit_flag = results->exit_flag;
        individual_results->model_concentrations =
                {results->model_concentrations.at(i)};
        individual_results->equilibrium_concentrations =
                {results->equilibrium_concentrations.at(i)};
        individual_results->measured_concentrations =
                {results->measured_concentrations.at(i)};
        
        const unsigned n_residuals = concentrations.at(i).size();
        individual_results->residuals.resize(n_residuals);
        individual_results->residual_gases.resize(n_residuals);
        for (unsigned j = 0; j < n_residuals; ++j, ++residuals_counter)
        {
            individual_results->residuals(j) =
                    results->residuals(residuals_counter);
            individual_results->residual_gases.at(j) =
                    results->residual_gases.at(residuals_counter);
        }
        function(individual_results,
                 std::vector<std::string>(1, sample_names.at(i)),
                 parameter_names_,
                 std::vector<SampleConcentrations>(1, concentrations.at(i)));
    }    
}

std::vector<unsigned> EnsembleResultsProcessorProxy::DetermineParameterIndices(
        unsigned n_shared_parameters,
        unsigned n_parameters,
        unsigned sample_number,
        const std::vector<std::string>& fit_parameter_names) const
{
    std::vector<unsigned> indices;
    for (const auto& parameter : parameter_names_)
    {
        auto it = std::find(fit_parameter_names.begin(),
                            fit_parameter_names.end(),
                            parameter);
        if (it != fit_parameter_names.end())
        {
            unsigned j = it - fit_parameter_names.begin();
            indices.push_back(j);;
        }
        else
        {
            it = std::find(fit_parameter_names.begin(),
                           fit_parameter_names.end(),
                           parameter +
                               boost::lexical_cast<std::string>(sample_number));
            if (it == fit_parameter_names.end())
                throw std::invalid_argument(
                        "This should never happen: parameter " + parameter +
                        boost::lexical_cast<std::string>(sample_number) +
                        " was not found.");
            
            unsigned j = it - fit_parameter_names.begin();
            indices.push_back(j);
        }
    }
    return indices;
}

Eigen::VectorXd EnsembleResultsProcessorProxy::SelectBestEstimates(
        const Eigen::VectorXd& all_estimates,
        const std::vector<unsigned>& parameter_indices) const
{
    unsigned selection_size = parameter_indices.size();
    Eigen::VectorXd selection(selection_size);
    for (unsigned i = 0; i < selection_size; ++i)
        selection(i) = all_estimates(parameter_indices[i]);
    return selection;
}

Eigen::MatrixXd EnsembleResultsProcessorProxy::SelectCovariances(
        const Eigen::MatrixXd& all_covariances,
        const std::vector<unsigned>& parameter_indices) const
{
    unsigned selection_size = parameter_indices.size();
    Eigen::MatrixXd selection(selection_size, selection_size);
    for (unsigned i = 0; i < selection_size; ++i)
        for (unsigned j = 0; j < selection_size; ++j)
            selection(i, j) = all_covariances(parameter_indices[i],
                                              parameter_indices[j]);
    return selection;
}
