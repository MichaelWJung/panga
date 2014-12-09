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


#ifndef CHI2PARAMETERCONFIGMODEL_H
#define CHI2PARAMETERCONFIGMODEL_H

#include <QModelIndex>

#include <qwt_interval.h>


class Chi2ParameterConfigModel : public QAbstractTableModel
{
    Q_OBJECT
    
public:
    explicit Chi2ParameterConfigModel(
        const std::vector<std::string>& paremeter_names,
        QObject* parent = nullptr);
    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index,
                         const QVariant& value,
                         int role = Qt::EditRole);
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    
    int GetXParameter() const;
    int GetYParameter() const;
    QString GetXParameterName() const;
    QString GetYParameterName() const;
    QwtInterval GetXInterval() const;
    QwtInterval GetYInterval() const;
    QwtInterval GetParameterInterval(int index) const;
    double GetValue(int index) const;
    std::vector<std::pair<int, double>> GetFixedParameters() const;
    
public slots:
    void SetValue(int index, double value);
    
signals:
    void AxesParametersChanged(int x_param, int y_param);
    void AxesParametersChanged(QString x_param, QString y_param);
    void AxesIntervalsChanged(QwtInterval x_interval, QwtInterval y_interval);
    void FixedParametersChanged(
            std::vector<std::pair<int, double>> fixed_parameters);
    void FixedParametersChanged();
    void FixedParameterValuesChanged();
    
private:
    void EmitAxesIntervalsChanged();
    void EmitAxesParametersChanged();
    void EmitFixedParameters();
    void EmitFixedParameterValues();
    
    enum class ColumnType
    {
        X_AXIS,
        Y_AXIS,
        IS_FITTED,
        MIN,
        MAX,
        VALUE
    };
    
    std::vector<ColumnType> columns_;
    std::map<ColumnType, int> columns_map_;
    std::vector<QString> parameter_names_;
    
    int x_param_;
    int y_param_;
    std::vector<bool> fitted_;
    std::vector<double> min_;
    std::vector<double> max_;
    std::vector<double> value_;
};

#endif // CHI2PARAMETERCONFIGMODEL_H
