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


#include <QBrush>

#include <cassert>

#include "core/misc/rundata.h"

#include "standardfitresultsmodel.h"

StandardFitResultsModel::StandardFitResultsModel(
        const std::vector<QString>& sample_names,
        const std::vector<ModelParameter>& parameters,
        const std::set<GasType>& gases_in_use,
        QObject* parent) :
    ResultsModel(
            sample_names,
            parameters,
            gases_in_use,
            parent),
    results_(std::make_shared<ResultsVector>()),
    column_types_(GetAvailableColumnTypes()),
    sample_needs_monte_carlo_initialized_(false)
{
}

StandardFitResultsModel::~StandardFitResultsModel()
{
}

void StandardFitResultsModel::ProcessResult(
        boost::shared_ptr<FitResults> results,
        const std::vector<std::string>& /*sample_names*/,
        const std::vector<std::string>& /*parameter_names*/,
        const std::vector<SampleConcentrations>& concentrations)
{
    assert(results->best_estimate.size() == n_parameters_);

    beginInsertRows(QModelIndex(), results_->size(), results_->size());
    results_->push_back(results);
    endInsertRows();

    sample_needs_monte_carlo_initialized_ = false;
}

int StandardFitResultsModel::columnCount(const QModelIndex& parent) const
{
    return column_types_.size();
}

QVariant StandardFitResultsModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return ResultsModel::headerData(section, orientation, role);

    try
    {
        return QVariant(GetColumnName(column_types_.at(section)));
    }
    catch (...)
    {
    }
    return QVariant();
}

QVariant StandardFitResultsModel::data(const QModelIndex& index, int role) const
{
    if (index == QModelIndex()) return QVariant();
    try
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return GetElement(*results_->at(index.row()),
                                  column_types_.at(index.column()));
            case Qt::ForegroundRole:
                return IsParameterInNormalRange(
                               *results_->at(index.row()),
                               column_types_.at(index.column())) ?
                           QBrush(Qt::black) :
                           QBrush(Qt::red);
            case Qt::BackgroundRole:
                return DoesSampleNeedMonteCarlo(index.row()) ?
                           QBrush(QColor(255, 255, 200)) :
                           QBrush(Qt::white);
            default:
                break;
        }
    }
    catch (...)
    {
    }
    return QVariant();
}

void StandardFitResultsModel::AddUnusedConcentrations(const RunData& run_data)
{
    assert(results_->size() == run_data.EnabledSize());
    auto input_concentrations = run_data.concentrations_begin();
    for (auto sample : *results_)
    {
        for (GasType gas = Gas::begin; gas != Gas::end; ++gas)
        {
            if (!sample->measured_concentrations.at(0).count(gas) &&
                input_concentrations->count(gas))
                sample->measured_concentrations.at(0)[gas] =
                        input_concentrations->at(gas);
        }
        ++input_concentrations;
    }
}

std::shared_ptr<const StandardFitResultsModel::ResultsVector>
StandardFitResultsModel::GetResultsVector() const
{
    return results_;
}

bool StandardFitResultsModel::IsParameterInNormalRange(
        const FitResults& results,
        const ExtendedColumnType& column_type) const
{
    unsigned i = column_type.second;
    switch (column_type.first)
    {
        case ColumnType::PARAMETER_ESTIMATE:
        {
            double val = GetElement(results, column_type).toDouble();
            return (val > parameters_.at(i).lowest_normal_value ) &&
                   (val < parameters_.at(i).highest_normal_value);
        }
        case ColumnType::PARAMETER_ESTIMATE_ERROR:
        {
            double val = GetElement(results, column_type).toDouble();
            return (val < parameters_.at(i).highest_normal_error) && (val > 0.);
        }
        default:
            return true;
    }
}

bool StandardFitResultsModel::DoesAnySampleNeedMonteCarlo() const
{
    bool monte_carlo_needed = false;
    for (unsigned i = 0; i < results_->size(); ++i)
        monte_carlo_needed |= DoesSampleNeedMonteCarlo(i);
    return monte_carlo_needed;
}

bool StandardFitResultsModel::DoesSampleNeedMonteCarlo(
        unsigned int sample_number) const
{
    if (!sample_needs_monte_carlo_initialized_)
    {
        sample_needs_monte_carlo_.clear();
        sample_needs_monte_carlo_.resize(results_->size(), false);
        for (const auto& column_type : column_types_)
        {
            if (column_type.first != ColumnType::PARAMETER_ESTIMATE &&
                column_type.first != ColumnType::PARAMETER_ESTIMATE_ERROR)
                continue;
            for (unsigned i = 0; i < results_->size(); ++i)
                sample_needs_monte_carlo_.at(i) =
                        sample_needs_monte_carlo_.at(i) |
                        !IsParameterInNormalRange(*results_->at(i),
                                                  column_type);
        }
        sample_needs_monte_carlo_initialized_ = true;
    }
    return sample_needs_monte_carlo_.at(sample_number);
}
