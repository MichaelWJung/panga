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


#include <boost/math/distributions.hpp>

#include <cassert>
#include <iterator>
#include <limits>

#include "resultsmodel.h"



ResultsModel::ResultsModel(const std::vector<QString>& sample_names,
                           const std::vector<ModelParameter>& parameters,
                           const std::set<GasType>& gases_in_use,
                           QObject* parent) :
    QAbstractTableModel(parent),
    sample_names_(sample_names),
    parameters_(parameters),
    gases_in_use_(gases_in_use),
    n_parameters_(parameters_.size())
{
}

ResultsModel::~ResultsModel()
{
}

int ResultsModel::rowCount(const QModelIndex& parent) const
{
    return sample_names_.size();
}

QVariant ResultsModel::headerData(int section,
                                  Qt::Orientation orientation,
                                  int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    try
    {
        if (orientation == Qt::Vertical)
            return QVariant(sample_names_.at(section));
    }
    catch (...)
    {
    }

    return QVariant();
}

const QList<ExtendedColumnType> ResultsModel::GetAvailableColumnTypes() const
{
    const unsigned n_covariances = (n_parameters_ * (n_parameters_ - 1) / 2);
    QList<ExtendedColumnType> cols;
    cols.push_back({ColumnType::DEGREES_OF_FREEDOM, 0});
    cols.push_back({ColumnType::CHI_SQUARE,         0});
    cols.push_back({ColumnType::PROBABILITY,        0});

    for (unsigned i = 0; i < n_parameters_; ++i)
    {
        cols.push_back({ColumnType::PARAMETER_ESTIMATE,       i});
        cols.push_back({ColumnType::PARAMETER_ESTIMATE_ERROR, i});
    }

    cols.push_back({ColumnType::CONVERGENCE,        0});

    for (unsigned i = 0; i < n_covariances; ++i)
        cols.push_back({ColumnType::CORRELATION, i});

    for (unsigned i = 0; i < gases_in_use_.size(); ++i)
        cols.push_back({ColumnType::RESIDUAL, i});

    for (unsigned i = 0; i < 6; ++i)
    {
        cols.push_back({ColumnType::EQUILIBRIUM_CONCENTRATION,       i});
        cols.push_back({ColumnType::EQUILIBRIUM_CONCENTRATION_ERROR, i});
    }

    for (unsigned i = 0; i < 6; ++i)
    {
        cols.push_back({ColumnType::MODEL_CONCENTRATION,       i});
        cols.push_back({ColumnType::MODEL_CONCENTRATION_ERROR, i});
    }

    for (unsigned i = 0; i < 5; ++i)
    {
        cols.push_back({ColumnType::MEASURED_CONCENTRATION,       i});
        cols.push_back({ColumnType::MEASURED_CONCENTRATION_ERROR, i});
    }

    cols.push_back({ColumnType::CHI, 0});

    return cols;
}

const QList<ExtendedColumnType> ResultsModel::GetDoubleColumnTypes() const
{
    auto available_column_types = GetAvailableColumnTypes();
    auto new_end = std::remove_if(
            available_column_types.begin(),
            available_column_types.end(),
            [](const ExtendedColumnType& c)
            {
                return (c.first == ColumnType::CONVERGENCE ||
                        c.first == ColumnType::DEGREES_OF_FREEDOM);
            });
    available_column_types.erase(new_end, available_column_types.end());

    return available_column_types;
}

QVariant ResultsModel::GetElement(
        const FitResults& results,
        const ExtendedColumnType& column_type) const
{
    try
    {
        unsigned i = column_type.second;
        switch (column_type.first)
        {
            case ColumnType::DEGREES_OF_FREEDOM:
                return results.degrees_of_freedom;
            case ColumnType::CHI_SQUARE:
                return results.chi_square;
            case ColumnType::CHI:
                return std::sqrt(results.chi_square);
            case ColumnType::PROBABILITY:
            {
                using boost::math::cdf;
                using boost::math::chi_squared;
                if (std::isnan(results.chi_square) ||
                    results.degrees_of_freedom <= 0)
                    return "nan";
                else
                    return 100. * (1. - cdf(
                        chi_squared(results.degrees_of_freedom),
                        results.chi_square));
            }
            case ColumnType::PARAMETER_ESTIMATE:
                return results.best_estimate(i);
            case ColumnType::PARAMETER_ESTIMATE_ERROR:
                return results.deviations.size() ?
                        results.deviations(i) :
                        std::numeric_limits<double>::quiet_NaN();
            case ColumnType::CONVERGENCE:
                return QString::fromStdString(results.GetExitFlagAsString());
            case ColumnType::CORRELATION:
            {
                const auto indices = GetCovarianceMatrixIndexes(i);
                return results.CorrelationCoefficient(indices.first,
                                                      indices.second);
            }
            case ColumnType::RESIDUAL:
                return GetResidualForGas(results, i);
            case ColumnType::EQUILIBRIUM_CONCENTRATION:
                return results.equilibrium_concentrations.at(0).at(
                        static_cast<GasType>(i)).value;
            case ColumnType::EQUILIBRIUM_CONCENTRATION_ERROR:
                return results.equilibrium_concentrations.at(0).at(
                        static_cast<GasType>(i)).error;
            case ColumnType::MODEL_CONCENTRATION:
                return results.model_concentrations.at(0).at(
                        static_cast<GasType>(i)).value;
            case ColumnType::MODEL_CONCENTRATION_ERROR:
                return results.model_concentrations.at(0).at(
                        static_cast<GasType>(i)).error;
            case ColumnType::MEASURED_CONCENTRATION:
                return results.measured_concentrations.at(0).at(
                        static_cast<GasType>(i)).value;
            case ColumnType::MEASURED_CONCENTRATION_ERROR:
                return results.measured_concentrations.at(0).at(
                        static_cast<GasType>(i)).error;
        }
    }
    catch (...)
    {
    }
    return QVariant();
}

QString ResultsModel::GetColumnName(const ExtendedColumnType& type) const
{
    unsigned i = type.second;
    switch (type.first)
    {
        case ColumnType::DEGREES_OF_FREEDOM:
            return "DoF";
        case ColumnType::CHI_SQUARE:
            return "Chi Square";
        case ColumnType::CHI:
            return "Chi";
        case ColumnType::PROBABILITY:
            return "Prob [%]";
        case ColumnType::PARAMETER_ESTIMATE:
            return parameters_.at(i).GetNameWithUnit();
        case ColumnType::PARAMETER_ESTIMATE_ERROR:
            return parameters_.at(i).GetSuffixedNameWithUnit("_err");
        case ColumnType::CONVERGENCE:
            return "Convergence";
        case ColumnType::CORRELATION:
        {
            auto indices = GetCovarianceMatrixIndexes(i);
            return "Corr_" + parameters_[indices.first ].GetName() +
                   "_"     + parameters_[indices.second].GetName();
        }
        case ColumnType::RESIDUAL:
            return QString::fromStdString("Res_" +
                    Gas::GasTypeToString(GetGasFromIndex(i)));
        case ColumnType::EQUILIBRIUM_CONCENTRATION:
        case ColumnType::EQUILIBRIUM_CONCENTRATION_ERROR:
        case ColumnType::MODEL_CONCENTRATION:
        case ColumnType::MODEL_CONCENTRATION_ERROR:
        case ColumnType::MEASURED_CONCENTRATION:
        case ColumnType::MEASURED_CONCENTRATION_ERROR:
        {
            QString gas = QString::fromStdString(Gas::GasTypeToString(
                    static_cast<GasType>(i)));
            switch (type.first)
            {
                case ColumnType::EQUILIBRIUM_CONCENTRATION:
                    return "Eq_" + gas + " [ccSTP/g]";
                case ColumnType::EQUILIBRIUM_CONCENTRATION_ERROR:
                    return "Eq_" + gas + "_err [ccSTP/g]";
                case ColumnType::MODEL_CONCENTRATION:
                    return "Mod_" + gas + " [ccSTP/g]";
                case ColumnType::MODEL_CONCENTRATION_ERROR:
                    return "Mod_" + gas + "_err [ccSTP/g]";
                case ColumnType::MEASURED_CONCENTRATION:
                    return "Meas_" + gas + " [ccSTP/g]";
                case ColumnType::MEASURED_CONCENTRATION_ERROR:
                    return "Meas_" + gas + "_err [ccSTP/g]";
                default:
                    return QString();
            }
        }
    }
    return QString();
}

std::pair<unsigned, unsigned>
ResultsModel::GetCovarianceMatrixIndexes(unsigned n)
{
    for (unsigned i = 1;; ++i)
        for (unsigned j = 0; j < i; ++j, --n)
            if (!n)
                return std::make_pair(j, i);
}

double ResultsModel::GetResidualForGas(const FitResults& results,
                                       unsigned gas_index) const
{
    GasType gas = GetGasFromIndex(gas_index);
    assert(results.residuals.size() == unsigned(results.residual_gases.size()));
    for (unsigned i = 0; i < results.residuals.size(); ++i)
        if (results.residual_gases[i] == gas)
            return results.residuals[i];
    return std::numeric_limits<double>::quiet_NaN();
}

GasType ResultsModel::GetGasFromIndex(unsigned i) const
{
    auto it = gases_in_use_.cbegin();
    std::advance(it, i);
    return *it;
}
