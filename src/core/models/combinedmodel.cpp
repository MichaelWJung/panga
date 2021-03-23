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


#include <stdexcept>

#include "combinedmodel.h"

CombinedModel::CombinedModel(ModelFactory* factory, CEqMethodFactory* ceqmethod_factory) :
    factory_(factory),
     ceqmethod_factory_(ceqmethod_factory),
     manager_(std::make_shared<ParameterManager>()),
    model_  (factory->CreateModel(manager_)),
     ceqmethod_ (ceqmethod_factory->CreateCEqMethod(manager_)),
     clever_ (std::make_shared<CleverMethod>(manager_)),
    model_accessor_ ( model_->GetParameterAccessor()),
     ceqmethod_accessor_ (ceqmethod_->GetParameterAccessor()),
     clever_accessor_ (clever_->GetParameterAccessor()),
    model_collector_( model_->GetDerivativeCollector()),
     ceqmethod_collector_( ceqmethod_->GetDerivativeCollector()),
     clever_collector_(clever_->GetDerivativeCollector()),
    parameters_(Eigen::VectorXd::Zero(manager_->GetParametersInOrder().size())),
    derivatives_(),
    cached_concentrations_()
{
}

void CombinedModel::SetParameters(const Eigen::VectorXd &parameters)
{
    if (unsigned(parameters.size()) != GetParametersInOrder().size())
        throw std::invalid_argument("The size of the supplied parameter vector and the number of "
                                    "parameters of this CombinedModel do not match.");

    model_accessor_->SetVectorReference(parameters);
    ceqmethod_accessor_->SetVectorReference(parameters);
    clever_accessor_->SetVectorReference(parameters);

    cached_concentrations_.clear();
    derivatives_.setZero();
}

void CombinedModel::SetupDerivatives(const std::vector<int> &indices)
{
    derivatives_.setZero(indices.size());
     model_collector_->SetDerivativesAndResultsVector(derivatives_, indices);
     ceqmethod_collector_->SetDerivativesAndResultsVector(derivatives_, indices);
    clever_collector_->SetDerivativesAndResultsVector(derivatives_, indices);
}

double CombinedModel::CalculateEquilibriumConcentration(GasType gas)
{
    return (gas == Gas::XE && ceqmethod_factory_->GetCEqMethodName() == "WeissClever") ? //WIP: Schöner lösen?
                clever_->CalculateConcentration(clever_accessor_, Gas::XE) :
                ceqmethod_ ->CalculateConcentration( ceqmethod_accessor_, gas); 
}

double CombinedModel::CalculateConcentration(GasType gas)
{
    const double& c_eq =
            cached_concentrations_[gas] = CalculateEquilibriumConcentration(gas);

    return model_->CalculateConcentration(c_eq, model_accessor_, gas);
}

const Eigen::RowVectorXd& CombinedModel::CalculateEquilibriumDerivatives(GasType gas)
{
    if (gas == Gas::XE && ceqmethod_factory_->GetCEqMethodName() == "WeissClever")
        clever_->CalculateDerivatives(clever_accessor_, clever_collector_, Gas::XE);
    else
        ceqmethod_ ->CalculateDerivatives( ceqmethod_accessor_,  ceqmethod_collector_, gas);

    return derivatives_;
}

const Eigen::RowVectorXd& CombinedModel::CalculateDerivatives(GasType gas)
{
    CalculateEquilibriumDerivatives(gas);

    double c_eq;
    if (!cached_concentrations_.count(gas))
        c_eq = cached_concentrations_[gas] =
                CalculateEquilibriumConcentration(gas);
    else
        c_eq = cached_concentrations_[gas];

    model_->CalculateDerivatives(c_eq, model_accessor_, model_collector_, gas);

    return derivatives_;
}

std::vector<ModelParameter> CombinedModel::GetParametersInOrder() const
{
    return manager_->GetParametersInOrder();
}

std::vector<std::string> CombinedModel::GetParameterNamesInOrder() const
{
    std::vector<ModelParameter> parameters(GetParametersInOrder());
    std::vector<std::string> ret;
    for (const auto& parameter : parameters)
        ret.push_back(parameter.name);
    return ret;   
}

std::set<std::string> CombinedModel::GetParameterNames() const
{
    std::vector<ModelParameter> parameters(GetParametersInOrder());
    std::set<std::string> ret;
    for (const auto& parameter : parameters)
        ret.insert(parameter.name);
    return ret;
}

unsigned CombinedModel::GetParameterIndex(const std::string &name) const
{
    try
    {
        return manager_->GetParameterIndex(name);
    }
    catch (ParameterManager::ParameterNotFound)
    {
        throw ParameterNotFound();
    }
}

std::shared_ptr<CombinedModel> CombinedModel::clone() const
{
    auto ret = std::make_shared<CombinedModel>(factory_, ceqmethod_factory_);
    ret->SetApplyConstraints(AreConstraintsApplied());
    return ret;
}

std::string CombinedModel::GetExcessAirModelName() const //WIP: Auch noch für Method einbauen, wo wird es verwendet?
{
    return  model_->GetModelName();
}

std::string CombinedModel::GetCEqMethodName() const 
{
    return  ceqmethod_->GetCEqMethodName();
}

void CombinedModel::SetApplyConstraints(bool apply)
{
    model_->SetApplyConstraints(apply);
}

bool CombinedModel::AreConstraintsApplied() const
{
    return model_->AreConstraintsApplied();
}
