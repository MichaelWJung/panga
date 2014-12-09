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


#ifndef MONTECARLOSUMMARYPROXYMODEL_H
#define MONTECARLOSUMMARYPROXYMODEL_H

#include <QAbstractTableModel>

#include <vector>

class MonteCarloResultsModel;

class MonteCarloSummaryProxyModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MonteCarloSummaryProxyModel(QObject* parent = 0);
    virtual ~MonteCarloSummaryProxyModel();

    virtual void SetSourceModel(MonteCarloResultsModel* new_source_model);

    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

private slots:
    void SourceDataChanged(const QModelIndex &source_top_left,
                           const QModelIndex &source_bottom_right);
    void SourceAboutToBeReset();
    void SourceReset();
    void SourceLayoutAboutToBeChanged();
    void SourceLayoutChanged();

private:
    void PopulateColumnsVector();

    enum class ColumnContentType { MEAN, ERROR, CORRELATION };
    struct ColumnType
    {
        ColumnType() = default;
        ColumnType(ColumnContentType content, unsigned original_column) :
            content(content), original_column(original_column)
        {
        }

        ColumnContentType content;
        unsigned original_column;
    };

    MonteCarloResultsModel* source_model_;
    std::vector<ColumnType> columns_;
};

#endif // MONTECARLOSUMMARYPROXYMODEL_H
