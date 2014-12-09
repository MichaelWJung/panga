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

#include "histogramdata1d.h"
#include "histogramdata2d.h"
#include "montecarloresultsmodel.h"

#include "montecarlosummaryproxymodel.h"

MonteCarloSummaryProxyModel::MonteCarloSummaryProxyModel(QObject* parent) :
    QAbstractTableModel(parent),
    source_model_(nullptr)
{
}

MonteCarloSummaryProxyModel::~MonteCarloSummaryProxyModel()
{
}

void MonteCarloSummaryProxyModel::SetSourceModel(
        MonteCarloResultsModel* new_source_model)
{
    beginResetModel();
    if (source_model_)
    {
        disconnect(source_model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                this, SLOT(SourceDataChanged(QModelIndex,QModelIndex)));
        disconnect(source_model_, SIGNAL(modelAboutToBeReset()),
                this, SLOT(SourceAboutToBeReset()));
        disconnect(source_model_, SIGNAL(modelReset()),
                this, SLOT(SourceReset()));
        disconnect(source_model_, SIGNAL(layoutAboutToBeChanged()),
                this, SLOT(SourceLayoutAboutToBeChanged()));
        disconnect(source_model_, SIGNAL(layoutChanged()),
                this, SLOT(SourceLayoutChanged()));
    }

    source_model_ = new_source_model;
    PopulateColumnsVector();

    connect(source_model_, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(SourceDataChanged(QModelIndex,QModelIndex)));
    connect(source_model_, SIGNAL(modelAboutToBeReset()),
            this, SLOT(SourceAboutToBeReset()));
    connect(source_model_, SIGNAL(modelReset()),
            this, SLOT(SourceReset()));
    connect(source_model_, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(SourceLayoutAboutToBeChanged()));
    connect(source_model_, SIGNAL(layoutChanged()),
            this, SLOT(SourceLayoutChanged()));

    endResetModel();
}

void MonteCarloSummaryProxyModel::PopulateColumnsVector()
{
    columns_.clear();

    if (source_model_->rowCount() < 1)
        return;

    std::vector<ColumnType> means_and_errors;
    std::vector<ColumnType> correlations;

    for (int i = 0; i < source_model_->columnCount(); ++i)
    {
        QModelIndex index = source_model_->index(0, i);
        if (index.data().canConvert<HistogramData>())
        {
            struct AddColumnVisitor : public boost::static_visitor<void>
            {
                AddColumnVisitor(
                        std::vector<ColumnType>& means_and_errors,
                        std::vector<ColumnType>& correlations,
                        unsigned i) :
                    means_and_errors(means_and_errors),
                    correlations(correlations),
                    i(i)
                {
                }

                void operator()(SharedHistogramData1D&) const
                {
                    means_and_errors.push_back(
                            ColumnType(ColumnContentType::MEAN, i));
                    means_and_errors.push_back(
                            ColumnType(ColumnContentType::ERROR, i));
                }

                void operator()(SharedHistogramData2D&) const
                {
                    correlations.push_back(
                            ColumnType(ColumnContentType::CORRELATION, i));
                }
            private:
                std::vector<ColumnType>& means_and_errors;
                std::vector<ColumnType>& correlations;
                unsigned i;
            } visitor(means_and_errors, correlations, i);
            try {
                HistogramData data = qvariant_cast<HistogramData>(index.data());
                boost::apply_visitor(visitor, data);
            }
            catch(...)
            {
            }
        }
    }

    std::move(means_and_errors.begin(),
              means_and_errors.end(),
              std::back_inserter(columns_));
    std::move(correlations.begin(),
              correlations.end(),
              std::back_inserter(columns_));
}

QVariant MonteCarloSummaryProxyModel::data(const QModelIndex& index,
                                           int role) const
{
    if (role == Qt::DisplayRole)
    {
        ColumnType column = columns_.at(index.column());
        QModelIndex source_index = source_model_->index(index.row(),
                                                        column.original_column);
        HistogramData data = qvariant_cast<HistogramData>(source_index.data());
        switch (column.content)
        {
            case ColumnContentType::MEAN:
                return boost::get<SharedHistogramData1D>(data)->CalcMean();
            case ColumnContentType::ERROR:
                return boost::get<SharedHistogramData1D>(data)->CalcStdDev();
            case ColumnContentType::CORRELATION:
                return boost::get<SharedHistogramData2D>(data)->CalcCorrelation();
        }
    }
    return QVariant();
}

QVariant MonteCarloSummaryProxyModel::headerData(
        int section,
        Qt::Orientation orientation,
        int role) const
{
    ColumnType column;
    if (orientation == Qt::Horizontal)
    {
        column = columns_.at(section);
        section = column.original_column;
    }
    QVariant header_data =
            source_model_->headerData(section, orientation, role);
    if (!header_data.canConvert<QString>() || orientation != Qt::Horizontal)
        return header_data;

    QString string = header_data.toString();
    switch (column.content)
    {
        case ColumnContentType::MEAN:
            string += " mean";
            break;
        case ColumnContentType::ERROR:
            string += " stddev";
            break;
        case ColumnContentType::CORRELATION:
            string += " corr";
            break;
    }

    return string;
}

int MonteCarloSummaryProxyModel::columnCount(const QModelIndex& parent) const
{
    return columns_.size();
}

int MonteCarloSummaryProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!source_model_)
        return 0;
    return source_model_->rowCount();
}

//! \todo Kann/will man das besser lösen?
void MonteCarloSummaryProxyModel::SourceDataChanged(
        const QModelIndex& source_top_left,
        const QModelIndex& source_bottom_right)
{
    if (rowCount() > 0 && columnCount() > 0)
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void MonteCarloSummaryProxyModel::SourceAboutToBeReset()
{
    beginResetModel();
}

void MonteCarloSummaryProxyModel::SourceReset()
{
    endResetModel();
}

void MonteCarloSummaryProxyModel::SourceLayoutAboutToBeChanged()
{
    emit layoutAboutToBeChanged();
}

void MonteCarloSummaryProxyModel::SourceLayoutChanged()
{
    PopulateColumnsVector();
    emit layoutChanged();
}
