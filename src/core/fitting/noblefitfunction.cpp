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


#include <cassert>
#include <cmath>
#include <stdexcept>

#include "fitresults.h"

#include "noblefitfunction.h"

NobleFitFunction::NobleFitFunction(
        std::shared_ptr<const CombinedModel> model,
        const NobleParameterMap& parameter_map,
        const std::vector<std::map<GasType, Data> >& concentrations
        ) :
    models_(concentrations.size()),
    parameter_map_(parameter_map),
    concentrations_(concentrations),
    n_concentrations_(NumberOfConcentrations())
{
    for (unsigned i = 0; i < models_.size(); ++i)
    {
        models_[i] = model->clone();
    }

    SetupDerivatives();

    if (concentrations_.size() != parameter_map_.GetNumberOfSamples())
        throw std::invalid_argument("The number of samples for which concentrations have been provided "
                                    "does not match the number of samples for which parameter "
                                    "mappings have been defined.");
}

NobleFitFunction::~NobleFitFunction()
{
}

void NobleFitFunction::SetParameters(const Eigen::VectorXd& parameters)
{
    parameters_ = parameter_map_.MapParameterValues(parameters);

    assert(parameters_.size() == concentrations_.size());

    for (unsigned i = 0; i < models_.size(); ++i)
        models_[i]->SetParameters(parameters_[i]);
}

void NobleFitFunction::CalcResiduals(Eigen::VectorXd& residuals) const
{
    residuals.resize(n_concentrations_);

    int i = 0;
    int k = 0;
    for (std::vector<std::map<GasType, Data> >::const_iterator it = concentrations_.begin();
         it != concentrations_.end();
         ++it, ++i)
    {
        for (std::map<GasType, Data>::const_iterator jt = it->begin();
             jt != it->end();
             ++jt, ++k)
        {
            residuals[k] = (jt->second.value - models_[i]->CalculateConcentration(jt->first)) /
                           jt->second.error;
        }
    }
}

void NobleFitFunction::CalcJacobian(Eigen::MatrixXd &jacobian) const
{
    jacobian.resize(n_concentrations_, parameter_map_.GetNumberOfFitParameters());

    int i = 0;
    int k = 0;
    for (std::vector<std::map<GasType, Data> >::const_iterator it = concentrations_.begin();
         it != concentrations_.end();
         ++it, ++i)
    {
        for (std::map<GasType, Data>::const_iterator jt = it->begin();
             jt != it->end();
             ++jt, ++k)
        {
            jacobian.row(k) = -models_[i]->CalculateDerivatives(jt->first) / jt->second.error;
        }
    }
}

void NobleFitFunction::CompileResults(
    std::shared_ptr<FitResults> results
    )
{
    results->equilibrium_concentrations.resize(concentrations_.size());
    results->      model_concentrations.resize(concentrations_.size());
    results->   measured_concentrations.resize(concentrations_.size());
    results->residual_gases.resize(results->residuals.size());

    SetParameters(results->best_estimate);

    unsigned residuals_counter = 0;
    for (unsigned i = 0; i < concentrations_.size(); ++i)
    {
        Eigen::MatrixXd equilibrium_jacobian(
                    static_cast<unsigned>(Gas::end_including_HE3),
                    parameter_map_.GetNumberOfFitParameters());

        Eigen::MatrixXd model_jacobian(
                    static_cast<unsigned>(Gas::end_including_HE3),
                    parameter_map_.GetNumberOfFitParameters());

        unsigned j = 0;
        for (GasType gas = Gas::begin;
             gas != Gas::end_including_HE3;
             ++gas, ++j)
        {
            results->equilibrium_concentrations[i][gas].value =
                    models_[i]->CalculateEquilibriumConcentration(gas);

            results->model_concentrations[i][gas].value =
                    models_[i]->CalculateConcentration(gas);

            equilibrium_jacobian.row(j) = models_[i]->CalculateEquilibriumDerivatives(gas);
            model_jacobian.row(j) = models_[i]->CalculateDerivatives(gas);

            if (concentrations_[i].count(gas))
                results->measured_concentrations[i][gas] = concentrations_[i][gas];
        }

        Eigen::MatrixXd equilibrium_covariances = equilibrium_jacobian *
                                            results->covariance_matrix *
                                            equilibrium_jacobian.transpose();

        Eigen::MatrixXd model_covariances = model_jacobian *
                                            results->covariance_matrix *
                                            model_jacobian.transpose();

        j = 0;
        for (GasType gas = Gas::begin;
             gas != Gas::end_including_HE3;
             ++gas, ++j)
        {
            results->equilibrium_concentrations[i][gas].error =
                    std::sqrt(equilibrium_covariances(j, j));

            results->model_concentrations[i][gas].error =
                    std::sqrt(model_covariances(j, j));
        }
        for (const auto& concentration : concentrations_[i])
            results->residual_gases[residuals_counter++] = concentration.first;
    }
}

std::shared_ptr<NobleFitFunction> NobleFitFunction::clone() const
{
    std::shared_ptr<NobleFitFunction> ret(std::make_shared<NobleFitFunction>(*this));

    for (unsigned i = 0; i < concentrations_.size(); ++i)
        ret->models_[i] = models_[i]->clone();

    return ret;
}

void NobleFitFunction::FixParameters(std::vector<std::pair<int, double> > parameters)
{
    parameter_map_.FixParameters(parameters);
    SetupDerivatives();
}

void NobleFitFunction::ResetParameters()
{
    parameter_map_.ResetParameters();
    SetupDerivatives();
}

void NobleFitFunction::SetupDerivatives()
{
    for (unsigned i = 0; i < models_.size(); ++i)
    {
        models_[i]->SetupDerivatives(parameter_map_.GetIndicesOfFittedParameters(i));
    }
}

unsigned NobleFitFunction::NumberOfConcentrations() const
{
    unsigned n = 0;

    for (std::vector<std::map<GasType, Data> >::const_iterator it = concentrations_.begin();
         it != concentrations_.end();
         ++it)
    {
        n += it->size();
    }

    return n;
}
