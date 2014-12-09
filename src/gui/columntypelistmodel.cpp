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


#include "columntypelistmodel.h"

ColumnTypeListModel::ColumnTypeListModel(
        QList<ExtendedColumnType> column_types,
        const MonteCarloResultsModel& results_model,
        bool move,
        bool ignore_drop_data,
        QObject* parent) :
    QAbstractListModel(parent),
    column_types_(),
    results_model_(results_model),
    ignore_drop_data_(ignore_drop_data)
{
    column_types_.reserve(column_types.size());
    for (const auto& i : column_types)
        column_types_.push_back(i);
    
    setSupportedDragActions(move ? Qt::MoveAction : Qt::CopyAction);
}

ColumnTypeListModel::ColumnTypeListModel(
        QList<ColumnTypeVariant> column_types,
        const MonteCarloResultsModel& results_model,
        bool move,
        bool ignore_drop_data,
        QObject* parent) :
    QAbstractListModel(parent),
    column_types_(column_types),
    results_model_(results_model),
    ignore_drop_data_(ignore_drop_data)
{
    setSupportedDragActions(move ? Qt::MoveAction : Qt::CopyAction);
}

QVariant ColumnTypeListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() >= rowCount()) return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            return results_model_.GetVariantColumnName(
                    column_types_.at(index.row()));
        case Qt::EditRole:
            return QVariant::fromValue(column_types_.at(index.row()));
        default:
            return QVariant();
    }
    
    return QVariant();
}

bool ColumnTypeListModel::setData(
        const QModelIndex& index,
        const QVariant& value,
        int role)
{
    if (role != Qt::EditRole) return false;
    if (!index.isValid()) return false;
    if (index.row() >= rowCount()) return false;
    
    column_types_[index.row()] =
            qvariant_cast<ColumnTypeVariant>(value);
            
    emit dataChanged(index, index);
    return true;
}

int ColumnTypeListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return column_types_.size();
}

Qt::ItemFlags ColumnTypeListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable  |
                      Qt::ItemIsDragEnabled |
                      Qt::ItemIsEnabled;
                      
    if (index.isValid())
    {
        try
        {   
            boost::get<ColumnType2D>(column_types_.at(index.row()));
        }
        catch (boost::bad_get)
        {
            f |= Qt::ItemIsDropEnabled;
        }
    }
    else
        f = Qt::ItemIsDropEnabled;
    
    return f;
}

bool ColumnTypeListModel::insertRows(int row, int count, const QModelIndex& parent)
{
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        column_types_.insert(row, ExtendedColumnType(
                {ColumnType::DEGREES_OF_FREEDOM, 0}));
    endInsertRows();
    return true;
}

bool ColumnTypeListModel::removeRows(int row, int count, const QModelIndex& parent)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        column_types_.removeAt(row);
    endRemoveRows();
    return true;
}

Qt::DropActions ColumnTypeListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData* ColumnTypeListModel::mimeData(const QModelIndexList& indexes) const
{
    ColumnTypeDragAndDropData* mimeData = new ColumnTypeDragAndDropData;
    mimeData->column_type =
            qvariant_cast<ColumnTypeVariant>(
                data(indexes.front(), Qt::EditRole));
            
    mimeData->setData("application/x-columntype", QByteArray());
    return mimeData;
}

QStringList ColumnTypeListModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-columntype";
    return types;
}

bool ColumnTypeListModel::dropMimeData(
        const QMimeData* data,
        Qt::DropAction action,
        int row,
        int /*column*/,
        const QModelIndex& parent)
{
    const ColumnTypeDragAndDropData* column_type_data =
            qobject_cast<const ColumnTypeDragAndDropData*>(data);
            
    if (!column_type_data) return false;
    if (ignore_drop_data_) return true;
    if (parent.isValid())
    {
        ExtendedColumnType drop_data;
        try
        {
            drop_data = boost::get<ExtendedColumnType>(
                    column_type_data->column_type);
        }
        catch (boost::bad_get)
        {
            return false;
        }
        ExtendedColumnType drop_target = boost::get<ExtendedColumnType>(
                column_types_.at(parent.row()));
        column_types_[parent.row()] = std::make_pair(drop_target, drop_data);
        emit dataChanged(parent, parent);
    }
    else
    {
        if (row < 0 || row > rowCount())
            row = rowCount();
        insertRow(row);
        setData(index(row), QVariant::fromValue(column_type_data->column_type));
    }
    return true;
}

const QList< ColumnTypeVariant >& ColumnTypeListModel::GetColumnTypes() const
{
    return column_types_;
}
