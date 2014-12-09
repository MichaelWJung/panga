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


#ifndef PARAMETERSETUPMODEL_H
#define PARAMETERSETUPMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

#include <boost/serialization/vector.hpp>

#include <string>
#include <vector>

#include "serializationhelpers.h"

class ParameterSetupModel : public QAbstractTableModel
{
    Q_OBJECT
    
public:
    explicit ParameterSetupModel(QObject* parent = 0);
    ParameterSetupModel(const ParameterSetupModel& other);
    
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role = Qt::EditRole);
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    
    double GetValue(QString parameter_name, unsigned sample_number) const;
    const QStringList& GetParameterNames() const;
    
    void MakeImmutable();
    
    struct ParameterNotInHeaderError
    {
        ParameterNotInHeaderError(QString parameter_name) :
            parameter_name(parameter_name) {}
        QString parameter_name;
    };
    
public slots:
    void AddParameter(QString name);
    void RemoveParameter(QString name);
    bool SetHeaderParameter(int section, const QString& parameter_name);
    void SetSampleNames(std::vector<std::string> new_sample_names);
    void DisableSample(unsigned index);
    void EnableSample(unsigned index);
    
signals:
    void LineNumberChanged();
    void ParameterAdded(QString name);
    void ParameterRemoved(QString name);
    
private:
    void InitializeValues();
    
    unsigned column_count_;
    unsigned row_count_;
    QStringList sample_names_;
    QStringList parameter_names_in_header_;
    QStringList parameter_names_;
    
    //! Die in der Tabelle gespeicherten Werte.
    /*!
     * Äußerer Vektor: Spalten; Innerer Vektor: Zeilen
     */
    std::vector<std::vector<double>> values_;
    std::vector<bool> samples_enabled_states_;
    
    bool immutable_;

    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template<class Archive>
void ParameterSetupModel::serialize(Archive& ar, const unsigned int version)
{
    ar & column_count_
       & row_count_
       & sample_names_
       & parameter_names_in_header_
       & parameter_names_
       & values_
       & samples_enabled_states_;
}

#endif // PARAMETERSETUPMODEL_H