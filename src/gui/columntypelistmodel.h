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


#ifndef COLUMNTYPELISTMODEL_H
#define COLUMNTYPELISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMimeData>

#include "montecarloresultsmodel.h"

class ColumnTypeListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    
    ColumnTypeListModel(QList<ExtendedColumnType> column_types,
                        const MonteCarloResultsModel& results_model,
                        bool move = false,
                        bool ignore_drop_data = false,
                        QObject* parent = 0);
    
    ColumnTypeListModel(QList<ColumnTypeVariant> column_types,
                        const MonteCarloResultsModel& results_model,
                        bool move = false,
                        bool ignore_drop_data = false,
                        QObject* parent = 0);
    
    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value,
                         int role = Qt::EditRole);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool insertRows(int row, int count,
                            const QModelIndex& parent = QModelIndex());
    virtual bool removeRows(int row, int count,
                            const QModelIndex& parent = QModelIndex());
    virtual Qt::DropActions supportedDropActions() const;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(
            const QMimeData* data,
            Qt::DropAction action,
            int row,
            int column,
            const QModelIndex& parent);
    const QList<ColumnTypeVariant>& GetColumnTypes() const;
    
private:
    QList<ColumnTypeVariant> column_types_;
    const MonteCarloResultsModel& results_model_;
    bool ignore_drop_data_;
};

class ColumnTypeDragAndDropData : public QMimeData
{
    Q_OBJECT
    
public:
    ColumnTypeVariant column_type;
};

#endif // COLUMNTYPELISTMODEL_H
