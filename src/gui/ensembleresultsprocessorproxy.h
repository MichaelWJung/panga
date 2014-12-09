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


#ifndef ENSEMBLERESULTSPROCESSORPROXY_H
#define ENSEMBLERESULTSPROCESSORPROXY_H

#include <Eigen/Core>

#include <memory>

#include "core/fitting/fitresultsprocessor.h"

class EnsembleResultsProcessorProxy : public FitResultsProcessor
{
public:
    EnsembleResultsProcessorProxy(
            const std::vector<std::string>& parameter_names,
            std::shared_ptr<FitResultsProcessor> processor);
    
    virtual void ProcessResult(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& fit_parameter_names,
            const std::vector<SampleConcentrations>& concentrations);
    virtual void ProcessMonteCarloResult(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& fit_parameter_names,
            const std::vector<SampleConcentrations>& concentrations);
    
private:
    template<typename F>
    void ConvertAndProcess(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& fit_parameter_names,
            const std::vector<SampleConcentrations>& concentrations,
            F function);
    
    //! \brief Bestimmt die Indices der Fitparameter die für die Probe
    //! sample_number relevant sind.
    std::vector<unsigned> DetermineParameterIndices(
            unsigned n_shared_parameters,
            unsigned n_parameters,
            unsigned sample_number,
            const std::vector<std::string>& fit_parameter_names) const;
    Eigen::VectorXd SelectBestEstimates(
            const Eigen::VectorXd& all_estimates,
            const std::vector<unsigned>& parameter_indices) const;
    Eigen::MatrixXd SelectCovariances(
            const Eigen::MatrixXd& all_covariances,
            const std::vector<unsigned>& parameter_indices) const;
            
    std::vector<std::string> parameter_names_;
    std::shared_ptr<FitResultsProcessor> processor_;
};

#endif // ENSEMBLERESULTSPROCESSORPROXY_H
