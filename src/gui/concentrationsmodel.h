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


#ifndef RUNDATAMODEL_H
#define RUNDATAMODEL_H

#include <QAbstractTableModel>
#include <QFile>

#include <boost/serialization/access.hpp>

#include "core/misc/rundata.h"

class QTextStream;

class ConcentrationsModel : public QAbstractTableModel
{
    Q_OBJECT
    
public:
    explicit ConcentrationsModel(QObject* parent = 0);
    ConcentrationsModel(const ConcentrationsModel& other);
    
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value,
                         int role = Qt::EditRole);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    
    void MakeImmutable();
    
    const RunData* GetRunData() const;
    
    void LoadSamplesFromFile();
    void LoadSamplesFromClipboard();
    
    void EmitSampleState();
    
public slots:
    void ChangeSampleState(Qt::CheckState state);
    
signals:
    void SamplesChanged(const std::vector<std::string>&);
    void SampleDisabled(unsigned index);
    void SampleEnabled(unsigned index);
    void SampleStateChanged(Qt::CheckState state);
    
private:
    void ConnectSignalsAndSlots();
    std::vector<Sample> ReadSamplesFromStream(QTextStream& stream) const;
    void LoadSamplesIntoModel(QTextStream& stream);
    Sample CreateSampleFromLine(const QString& line) const;
    bool ContainsDuplicateSampleNames(const std::vector<Sample>& samples) const;
    
private slots:
    void EmitSampleEnabled(unsigned index);
    void EmitSampleDisabled(unsigned index);
    
private:
    RunData run_data_;
    bool immutable_;
    
    
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned version)
    {
        ar & run_data_;
    }
};

#endif // RUNDATAMODEL_H