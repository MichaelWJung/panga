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


#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "core/misc/templates.h"

#include "parametersetupmodel.h"

ParameterSetupModel::ParameterSetupModel(QObject* parent) :
    QAbstractTableModel(parent),
    column_count_(0U),
    row_count_(0U),
    sample_names_(),
    parameter_names_in_header_(),
    parameter_names_(),
    values_(),
    samples_enabled_states_(),
    immutable_(false)
{
}

ParameterSetupModel::ParameterSetupModel(const ParameterSetupModel& other) :
    QAbstractTableModel(static_cast<const QObject&>(other).parent()),
    column_count_(other.column_count_),
    row_count_(other.row_count_),
    sample_names_(other.sample_names_),
    parameter_names_in_header_(other.parameter_names_in_header_),
    parameter_names_(other.parameter_names_),
    values_(other.values_),
    samples_enabled_states_(other.samples_enabled_states_),
    immutable_(false)
{
}

int ParameterSetupModel::columnCount(const QModelIndex& parent) const
{
    return column_count_;
}

int ParameterSetupModel::rowCount(const QModelIndex& parent) const
{
    return row_count_;
}

Qt::ItemFlags ParameterSetupModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags;
    flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (!immutable_)
        flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant ParameterSetupModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        try
        {
            return values_.at(index.column()).at(index.row());
        }
        catch (std::out_of_range)
        {
        }
    }
    
    return QVariant();
}

bool ParameterSetupModel::setData(
        const QModelIndex& index,
        const QVariant& value,
        int role)
{
    if (role != Qt::EditRole) return false;
    if (immutable_) return false;
    bool ok = true;
    double val = value.toDouble(&ok);
    if (!ok)
        return false;
    
    try
    {
        values_.at(index.column()).at(index.row()) = val;
        emit dataChanged(index, index);
    }
    catch (std::out_of_range)
    {
    }
    return false;
}

QVariant ParameterSetupModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (unsigned(section) < column_count_)
                return parameter_names_in_header_[section];
        }
        else
            if (section < sample_names_.size())
                return sample_names_.at(section);
    }
    
    return QVariant();
}

bool ParameterSetupModel::SetHeaderParameter(
    int section,
    const QString& parameter_name)
{
    if (immutable_) return false;
    if (unsigned(section) >= column_count_) return false;
    if (parameter_name == "")
    {
        parameter_names_in_header_[section] = "";
        emit headerDataChanged(Qt::Horizontal, section, section);
        return true;
    }
        
    if (!parameter_names_.contains(parameter_name))
        return false;
    
    int old_index = parameter_names_in_header_.indexOf(parameter_name);
    if (old_index >= 0 && old_index != section)
    {
        parameter_names_in_header_.swap(section, old_index);
        emit headerDataChanged(Qt::Horizontal, old_index, old_index);
    }
    else
        parameter_names_in_header_[section] = parameter_name;
    emit headerDataChanged(Qt::Horizontal, section, section);
    return true;
}

void ParameterSetupModel::AddParameter(QString name)
{
    if (parameter_names_.contains(name)) return;
    parameter_names_.push_back(name);
    while (unsigned(parameter_names_.size()) > column_count_)
    {
        beginInsertColumns(QModelIndex(), column_count_, column_count_);
        parameter_names_in_header_.push_back(name);
        ++column_count_;
        values_.push_back(std::vector<double>(row_count_, 0.0));
        endInsertColumns();
    }
    emit ParameterAdded(name);
}

void ParameterSetupModel::RemoveParameter(QString name)
{
    parameter_names_.removeAll(name);
    int i = 0;
    while ((i = parameter_names_in_header_.indexOf(name, i)) >= 0)
    {
        parameter_names_in_header_.replace(i, "");
        emit headerDataChanged(Qt::Horizontal, i, i);
    }
    emit ParameterRemoved(name);
}

double ParameterSetupModel::GetValue(QString parameter_name,
                                     unsigned int sample_number) const
{
    int i = parameter_names_in_header_.indexOf(parameter_name);
    if (i < 0) throw ParameterNotInHeaderError(parameter_name);
         
    unsigned j = TransformSequenceIndexToSkipElements(
            samples_enabled_states_, sample_number, [](bool b) { return b; });
        
    return values_.at(i).at(j);
}

const QStringList& ParameterSetupModel::GetParameterNames() const
{
    return parameter_names_;
}

void ParameterSetupModel::MakeImmutable()
{
    immutable_ = true;
}

void ParameterSetupModel::SetSampleNames(
        std::vector<std::string> new_sample_names)
{
    if (!sample_names_.isEmpty())
    {
        beginRemoveRows(QModelIndex(), 0, sample_names_.size() - 1);
        sample_names_.clear();
        endRemoveRows();
    }
    values_.clear();
    if (!new_sample_names.empty())
    {
        beginInsertRows(QModelIndex(), 0, new_sample_names.size() - 1);
        for (const auto& sample_name : new_sample_names)
            sample_names_.push_back(QString::fromStdString(sample_name));
        row_count_ = sample_names_.size();
        InitializeValues();
        endInsertRows();
    }
    else
        row_count_ = 0;
    samples_enabled_states_ = std::vector<bool>(new_sample_names.size(), true);
    emit LineNumberChanged();
}

void ParameterSetupModel::InitializeValues()
{
    values_ = std::vector<std::vector<double>>(
            column_count_,
            std::vector<double>(row_count_, 0.0));
}

void ParameterSetupModel::DisableSample(unsigned int index)
{
    samples_enabled_states_.at(index) = false;
}

void ParameterSetupModel::EnableSample(unsigned int index)
{
    samples_enabled_states_.at(index) = true;
}
