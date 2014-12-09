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


#include "chi2parameterconfigmodel.h"

Chi2ParameterConfigModel::Chi2ParameterConfigModel(
        const std::vector<std::string>& parameter_names,
        QObject* parent) :
    QAbstractTableModel(parent),
    columns_({ColumnType::X_AXIS,
              ColumnType::Y_AXIS,
              ColumnType::IS_FITTED,
              ColumnType::MIN,
              ColumnType::MAX,
              ColumnType::VALUE}),
    columns_map_(),
    parameter_names_(),
    x_param_(0),
    y_param_(1),
    fitted_(parameter_names.size(), true),
    min_   (parameter_names.size(), 0.),
    max_   (parameter_names.size(), 2.),
    value_ (parameter_names.size(), 1.)
{
    for (const auto& p : parameter_names)
        parameter_names_.push_back(QString::fromStdString(p));
    for (unsigned i = 0; i < columns_.size(); ++i)
        columns_map_[columns_[i]] = i;
}

QVariant Chi2ParameterConfigModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (columns_.at(index.column()))
        {
            case ColumnType::X_AXIS:
            case ColumnType::Y_AXIS:
            case ColumnType::IS_FITTED:
                return QVariant();
            case ColumnType::MIN:
                return min_.at(row);
            case ColumnType::MAX:
                return max_.at(row);
            case ColumnType::VALUE:
                return value_.at(row);
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        switch (columns_.at(index.column()))
        {
            case ColumnType::X_AXIS:
                return row == x_param_ ? Qt::Checked : Qt::Unchecked;
            case ColumnType::Y_AXIS:
                return row == y_param_ ? Qt::Checked : Qt::Unchecked;
            case ColumnType::IS_FITTED:
                if (row == x_param_ || row == y_param_)
                    return QVariant();
                return fitted_.at(row) ? Qt::Checked : Qt::Unchecked;
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool Chi2ParameterConfigModel::setData(const QModelIndex& ind,
                                       const QVariant& value,
                                       int role)
{
    if (role == Qt::EditRole)
    {
        switch (columns_.at(ind.column()))
        {
            case ColumnType::VALUE:
                SetValue(ind.row(), value.toDouble());
                return true;
            default:
                return false;
                
            case ColumnType::MIN:
                if (min_.at(ind.row()) == value.toDouble())
                    return true;
                min_.at(ind.row()) = value.toDouble();
                break;
            case ColumnType::MAX:
                if (max_.at(ind.row()) == value.toDouble())
                    return true;
                max_.at(ind.row()) = value.toDouble();
                break;
        }
        emit dataChanged(ind, ind);
        if (ind.row() == x_param_ || ind.row() == y_param_)
                EmitAxesIntervalsChanged();
        EmitFixedParameters();
        return true;
    }
    else if (role == Qt::CheckStateRole)
    {
        switch (columns_.at(ind.column()))
        {
            case ColumnType::IS_FITTED:
                fitted_.at(ind.row()) = value.toBool();
                EmitFixedParameters();
                break;
            case ColumnType::X_AXIS:
                if (!value.toBool())
                    return false;
                if (ind.row() == y_param_)
                    std::swap(x_param_, y_param_);
                else
                    x_param_ = ind.row();
                EmitAxesIntervalsChanged();
                EmitAxesParametersChanged();
                break;
            case ColumnType::Y_AXIS:
                if (!value.toBool())
                    return false;
                if (ind.row() == x_param_)
                    std::swap(x_param_, y_param_);
                else
                    y_param_ = ind.row();
                EmitAxesIntervalsChanged();
                EmitAxesParametersChanged();
                break;
            default:
                return false;
        }
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
        return true;
    }
    return false;
}

QVariant Chi2ParameterConfigModel::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (columns_.at(section))
            {
                case ColumnType::X_AXIS:
                    return QVariant("X");
                case ColumnType::Y_AXIS:
                    return QVariant("Y");
                case ColumnType::IS_FITTED:
                    return QVariant("fit?");
                case ColumnType::MIN:
                    return QVariant("min");
                case ColumnType::MAX:
                    return QVariant("max");
                case ColumnType::VALUE:
                    return QVariant("value");
            }
        }
        else
            return QVariant(parameter_names_.at(section));
    }
    return QVariant();
}

int Chi2ParameterConfigModel::columnCount(const QModelIndex& parent) const
{
    return columns_.size();
}

int Chi2ParameterConfigModel::rowCount(const QModelIndex& parent) const
{
    return parameter_names_.size();
}

Qt::ItemFlags Chi2ParameterConfigModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f;
    switch (columns_.at(index.column()))
    {
        case ColumnType::IS_FITTED:
            if (index.row() == x_param_ || index.row() == y_param_)
                break;
        case ColumnType::X_AXIS:
        case ColumnType::Y_AXIS:
            f |= Qt::ItemIsUserCheckable;
            f |= Qt::ItemIsEnabled;
            break;
        case ColumnType::MIN:
        case ColumnType::MAX:
            if (index.row() == x_param_ || index.row() == y_param_ ||
                !fitted_.at(index.row()))
            {
                f |= Qt::ItemIsEditable;
                f |= Qt::ItemIsEnabled;
            }
            break;
        case ColumnType::VALUE:
            if (index.row() != x_param_ && index.row() != y_param_ &&
                !fitted_.at(index.row()))
            {
                f |= Qt::ItemIsEditable;
                f |= Qt::ItemIsEnabled;
            }
            break;
    }
    return f;
}

int Chi2ParameterConfigModel::GetXParameter() const
{
    return x_param_;
}

int Chi2ParameterConfigModel::GetYParameter() const
{
    return y_param_;
}

QString Chi2ParameterConfigModel::GetXParameterName() const
{
    return parameter_names_.at(x_param_);
}

QString Chi2ParameterConfigModel::GetYParameterName() const
{
    return parameter_names_.at(y_param_);
}

QwtInterval Chi2ParameterConfigModel::GetXInterval() const
{
    return QwtInterval(min_.at(x_param_), max_.at(x_param_));
}

QwtInterval Chi2ParameterConfigModel::GetYInterval() const
{
    return QwtInterval(min_.at(y_param_), max_.at(y_param_));
}

QwtInterval Chi2ParameterConfigModel::GetParameterInterval(int index) const
{
    return QwtInterval(min_.at(index), max_.at(index));
}

double Chi2ParameterConfigModel::GetValue(int index) const
{
    return value_.at(index);
}

std::vector<std::pair<int, double>>
Chi2ParameterConfigModel::GetFixedParameters() const
{
    std::vector<std::pair<int,double>> fixed_parameters;
    for (unsigned i = 0; i < fitted_.size(); ++i)
        if (!fitted_.at(i))
            fixed_parameters.push_back(std::make_pair(i, value_.at(i)));
    return fixed_parameters;
}

void Chi2ParameterConfigModel::SetValue(int row, double value)
{
    if (value_.at(row) != value)
    {
        value = std::max(value, min_.at(row));
        value = std::min(value, max_.at(row));
        value_.at(row) = value;
        emit dataChanged(index(row, columns_map_.at(ColumnType::VALUE)),
                         index(row, columns_map_.at(ColumnType::VALUE)));
        EmitFixedParameterValues();
    }
}

void Chi2ParameterConfigModel::EmitAxesIntervalsChanged()
{
    emit AxesIntervalsChanged(GetXInterval(), GetYInterval());
}

void Chi2ParameterConfigModel::EmitAxesParametersChanged()
{
    emit AxesParametersChanged(x_param_, y_param_);
    emit AxesParametersChanged(parameter_names_.at(x_param_),
                               parameter_names_.at(y_param_));
}

void Chi2ParameterConfigModel::EmitFixedParameters()
{
    emit FixedParametersChanged();
    emit FixedParametersChanged(GetFixedParameters());
}

void Chi2ParameterConfigModel::EmitFixedParameterValues()
{
    emit FixedParameterValuesChanged();
    emit FixedParametersChanged(GetFixedParameters());
}
