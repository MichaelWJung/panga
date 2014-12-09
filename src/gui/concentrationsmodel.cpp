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


#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include <algorithm>
#include <iostream>
#include <iterator>

#include "commons.h"

#include "concentrationsmodel.h"

ConcentrationsModel::ConcentrationsModel(QObject* parent) :
    QAbstractTableModel(parent),
    run_data_(),
    immutable_(false)
{
    ConnectSignalsAndSlots();
}

ConcentrationsModel::ConcentrationsModel(const ConcentrationsModel& other) :
    QAbstractTableModel(static_cast<const QObject&>(other).parent()),
    run_data_(other.run_data_),
    immutable_(false)
{
    ConnectSignalsAndSlots();
}

void ConcentrationsModel::ConnectSignalsAndSlots()
{
    connect(&run_data_, SIGNAL(SampleEnabled(unsigned)),
            this, SLOT(EmitSampleEnabled(unsigned)));
    connect(&run_data_, SIGNAL(SampleDisabled(unsigned)),
            this, SLOT(EmitSampleDisabled(unsigned)));
}

void ConcentrationsModel::LoadSamplesFromFile()
{
    QString filename = QFileDialog::getOpenFileName(
            nullptr,
            "Load Samples",
            QString(),
            "CSV files (*.csv);;All Files (*)");
    if (filename.isEmpty())
        return;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;
    
    QString contents(file.readAll());
    QRegExp rx("\\r(?!\\n)");
    contents.replace(rx, "\n");
    QTextStream stream(&contents);
    LoadSamplesIntoModel(stream);
}

void ConcentrationsModel::LoadSamplesFromClipboard()
{
    QString clipboard_text = QApplication::clipboard()->text();
    QRegExp rx("\\r(?!\\n)");
    clipboard_text.replace(rx, "\n");
    QTextStream stream(&clipboard_text);
    LoadSamplesIntoModel(stream);
}

void ConcentrationsModel::EmitSampleState()
{
    Qt::CheckState check_state;
    if (run_data_.EnabledSize() == run_data_.TotalSize())
        check_state = Qt::Checked;
    else if (run_data_. EnabledSize() == 0)
        check_state = Qt::Unchecked;
    else
        check_state = Qt::PartiallyChecked;
    
    emit SampleStateChanged(check_state);
}

std::vector<Sample> ConcentrationsModel::ReadSamplesFromStream(
        QTextStream& stream) const
{
    std::vector<Sample> samples;
    while (!stream.atEnd())
    {
        try
        {
            samples.emplace_back(CreateSampleFromLine(stream.readLine()));
        }
        catch (std::invalid_argument)
        {
            continue;
        }
    }
    return samples;
}

void ConcentrationsModel::LoadSamplesIntoModel(QTextStream& stream)
{
    std::vector<Sample> samples(ReadSamplesFromStream(stream));
    if (samples.empty())
    {
        QMessageBox::warning(nullptr, APPLICATION_NAME,
                             "Error: Invalid input format.");
        return;
    }
    if (ContainsDuplicateSampleNames(samples)) 
    {
        QMessageBox::critical(nullptr, APPLICATION_NAME,
                              "Error: duplicate sample names are not allowed.");
        return;
    }
    
    std::vector<std::string> old_sample_names = run_data_.GetAllSampleNames();
    
    beginResetModel();
    run_data_.clear();
    for(auto& sample : samples)
        run_data_.Add(std::move(sample));
    endResetModel();
    
    std::vector<std::string> new_sample_names = run_data_.GetAllSampleNames();
    if (old_sample_names != new_sample_names)
        emit SamplesChanged(new_sample_names);
    
    emit SampleStateChanged(Qt::Checked);
}

Sample ConcentrationsModel::CreateSampleFromLine(const QString& line) const
{
    QStringList strings;
    if (!((strings = line.split(',' )).size() == 11 ||
          (strings = line.split('\t')).size() == 11))
        throw std::invalid_argument("A line must consist of the sample name "
                                    "and the following concentrations, all "
                                    "separated by commas or tabs: He, He err, "
                                    "Ne, Ne err, Ar, Ar err, Kr, Kr err, Xe, "
                                    "Xe err.");
    
    std::string sample_name = strings.takeFirst().toStdString();
    SampleConcentrations concentrations;
    
    for (unsigned i = 0; i < 5; ++i)
    {
        bool value_ok = true;
        bool error_ok = true;
        double value = strings.takeFirst().toDouble(&value_ok);
        double error = strings.takeFirst().toDouble(&error_ok);
        
        if (!value_ok || !error_ok)
            continue;
        
        concentrations[static_cast<GasType>(i)] = Data(value, error);
    }
    
    return std::make_pair(sample_name, concentrations);
}

bool ConcentrationsModel::ContainsDuplicateSampleNames(
        const std::vector<Sample>& samples) const
{
    std::vector<std::string> sample_names;
    std::transform(samples.cbegin(),
                   samples.cend(),
                   std::back_inserter(sample_names),
                   [](const Sample& sample) { return sample.first; });
    std::sort(sample_names.begin(), sample_names.end());
    return (std::unique(sample_names.begin(), sample_names.end())
            != sample_names.end());
}

Qt::ItemFlags ConcentrationsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags;
    if (!index.column())
        flags |= Qt::ItemIsUserCheckable;
    else
        flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (!immutable_)
        flags |= Qt::ItemIsEnabled;
    return flags;
}

QVariant ConcentrationsModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int col = index.column();
        if (!col--) return QVariant();
        const SampleConcentrations& sample =
                run_data_.GetSampleConcentrations(index.row());
        
        GasType gas = static_cast<GasType>(col / 2);
        
        auto it = sample.find(gas);
        if (it != sample.end())
            return col % 2 ? it->second.error : it->second.value;
    }
    if (role == Qt::CheckStateRole && !index.column())
    {
        return run_data_.IsEnabled(index.row()) ? 2 : 0;
    }
    
    return QVariant();
}

QVariant ConcentrationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
        switch (orientation)
        {
            case Qt::Horizontal:
                if (!section--) return QVariant();
                if (!(section % 2))
                    return Gas::GasTypeToString(
                            static_cast<GasType>(section / 2)).c_str();
                else
                    return "err";
                break;
                
            case Qt::Vertical:
                return run_data_.GetSampleName(section).c_str();
                break;
        }
    return QVariant();
}

bool ConcentrationsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column())
        return QAbstractItemModel::setData(index, value, role);
    if (role != Qt::CheckStateRole) return false;
    const unsigned sample_index = index.row();
    if (value.toBool())
    {
        run_data_.EnableSample(sample_index);
        emit SampleEnabled(sample_index);
        if (run_data_.EnabledSize() == run_data_.TotalSize())
            emit SampleStateChanged(Qt::Checked);
        else if (run_data_.EnabledSize() == 1)
            emit SampleStateChanged(Qt::PartiallyChecked);
    }
    else
    {
        run_data_.DisableSample(sample_index);
        emit SampleDisabled(sample_index);
        if (run_data_.EnabledSize() == 0)
            emit SampleStateChanged(Qt::Unchecked);
        else if (run_data_.EnabledSize() == run_data_.TotalSize() - 1)
            emit SampleStateChanged(Qt::PartiallyChecked);
    }
    return true;
}

int ConcentrationsModel::rowCount(const QModelIndex& parent) const
{
    return run_data_.TotalSize();
}

int ConcentrationsModel::columnCount(const QModelIndex& parent) const
{
    return 11;
}

void ConcentrationsModel::MakeImmutable()
{
    immutable_ = true;
}

const RunData* ConcentrationsModel::GetRunData() const
{
    return &run_data_;
}

void ConcentrationsModel::ChangeSampleState(Qt::CheckState state)
{
    switch (state)
    {
        case Qt::Checked:
            run_data_.EnableAllSamples();
            emit SampleStateChanged(Qt::Checked);
            emit dataChanged(createIndex(0, 0),
                             createIndex(rowCount(QModelIndex()), 0));
            break;
        case Qt::Unchecked:
            run_data_.DisableAllSamples();
            emit SampleStateChanged(Qt::Unchecked);
            emit dataChanged(createIndex(0, 0),
                             createIndex(rowCount(QModelIndex()), 0));
            break;
        default:
            ;
    }
}

void ConcentrationsModel::EmitSampleEnabled(unsigned int index)
{
    emit SampleEnabled(index);
}

void ConcentrationsModel::EmitSampleDisabled(unsigned int index)
{
    emit SampleDisabled(index);
}
