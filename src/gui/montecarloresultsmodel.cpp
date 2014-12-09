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


#include <boost/make_shared.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <set>

#include "core/misc/gas.h"

#include "histogramdata1d.h"
#include "histogramdata2d.h"
#include "montecarloexporter.h"

#include "montecarloresultsmodel.h"

MonteCarloResultsModel::GetHeaderNameVisitor::GetHeaderNameVisitor(
        const MonteCarloResultsModel& model) :
    model_(model)
{
}

QString MonteCarloResultsModel::GetHeaderNameVisitor::operator()(
        const ExtendedColumnType& type) const
{
    return model_.GetColumnName(type);
}

QString MonteCarloResultsModel::GetHeaderNameVisitor::operator()(
        const ColumnType2D& type) const
{
    return model_.GetColumnName(type.second) + " vs " +
           model_.GetColumnName(type.first);
}

MonteCarloResultsModel::GetAxisTitlesVisitor::GetAxisTitlesVisitor(
        const MonteCarloResultsModel& model) :
    model_(model)
{
}

QStringList MonteCarloResultsModel::GetAxisTitlesVisitor::operator()(
        const ExtendedColumnType& type) const
{
    QStringList titles;
    titles += model_.GetColumnName(type);
    return titles;
}

QStringList MonteCarloResultsModel::GetAxisTitlesVisitor::operator()(
        const ColumnType2D& type) const
{
    QStringList titles;
    titles += model_.GetColumnName(type.second);
    titles += model_.GetColumnName(type.first);
    return titles;
}

MonteCarloResultsModel::GetOrCreateDataVisitor::GetOrCreateDataVisitor(
        const MonteCarloResultsModel& model, const QModelIndex& index) :
    model_(model), index_(index)
{
}

HistogramData MonteCarloResultsModel::GetOrCreateDataVisitor::operator()(
        const ExtendedColumnType& type) const
{
    if (!model_.plot_data_1d_.at(index_.row()).count(type))
        CreatePlotData(type);

    auto& data = model_.plot_data_1d_.at(index_.row()).at(type);
    return data;
}

void MonteCarloResultsModel::GetOrCreateDataVisitor::CreatePlotData(
        const ExtendedColumnType& type) const
{
    SharedHistogramData1D& data = model_.plot_data_1d_.at(index_.row())[type];
    data = boost::make_shared<HistogramData1D>(
            model_.n_monte_carlos_,
            model_.n_bins_,
            model_.masks_.at(index_.row()),
            model_.monte_carlo_data_.at(index_.row()).at(type));
    data->SetOriginalResult(model_.GetElement(
            *model_.original_fit_results_->at(index_.row()), type).toDouble());
    model_.AddToDataToIndexMapAndConnectSignals(
        data.get(), index_.row(), index_.column());
}

HistogramData MonteCarloResultsModel::GetOrCreateDataVisitor::operator()(
        const ColumnType2D& type) const
{
    if (!model_.plot_data_2d_.at(index_.row()).count(type))
        CreatePlotData(type);

    auto& data = model_.plot_data_2d_.at(index_.row()).at(type);
    return data;
}

void MonteCarloResultsModel::GetOrCreateDataVisitor::CreatePlotData(
        const ColumnType2D& type) const
{
    auto& mc_data_x = model_.monte_carlo_data_.at(index_.row()).at(type.first);
    auto& mc_data_y = model_.monte_carlo_data_.at(index_.row()).at(type.second);
    assert(mc_data_x->vector_->size() == mc_data_y->vector_->size());
    SharedHistogramData2D& data = model_.plot_data_2d_.at(index_.row())[type];
    data = boost::make_shared<HistogramData2D>(
            model_.n_monte_carlos_,
            model_.n_bins_,
            model_.masks_.at(index_.row()),
            mc_data_x,
            mc_data_y);
    data->SetOriginalResult(
            model_.GetElement(*model_.original_fit_results_
                ->at(index_.row()), type.first ).toDouble(),
            model_.GetElement(*model_.original_fit_results_
                ->at(index_.row()), type.second).toDouble());
    model_.AddToDataToIndexMapAndConnectSignals(
        data.get(), index_.row(), index_.column());
}

MonteCarloResultsModel::MonteCarloResultsModel(
        const std::vector<QString>& sample_names,
        const std::vector<ModelParameter>& parameters,
        const std::set<GasType>& gases_in_use,
        unsigned n_monte_carlos,
        QObject* parent) :
    ResultsModel(
            sample_names,
            parameters,
            gases_in_use,
            parent),
    column_types_(PrepareColumnTypes(parameters.size(),
                                     gases_in_use.size())),
    available_column_types_(GetDoubleColumnTypes()),
    n_monte_carlos_(n_monte_carlos),
    n_bins_(0U),
    monte_carlo_data_(sample_names_.size()),
    name_lookup_([&]()->std::map<std::string, unsigned>
                 {
                     std::map<std::string, unsigned> name_lookup;
                     for (unsigned i = 0; i < sample_names_.size(); ++i)
                         name_lookup[sample_names_[i].toStdString()] = i;
                     return name_lookup;
                 }()),
    plot_data_1d_(sample_names_.size()),
    plot_data_2d_(sample_names_.size()),
    masks_(sample_names_.size()),
    exit_flags_(sample_names_.size()),
    original_fit_results_need_to_be_added_to_plot_data_vectors_(false)
{
    for (unsigned i = 0; i < sample_names_.size(); ++i)
    {
        exit_flags_[i] = boost::make_shared<std::vector<Eigen::LM::Status>>();
        exit_flags_[i]->reserve(n_monte_carlos_);
        masks_[i] = boost::make_shared<Mask>(n_monte_carlos, exit_flags_[i]);
        for (const auto& col_type : available_column_types_)
        {
            auto& new_col = monte_carlo_data_[i][col_type];
            new_col = boost::make_shared<DataVector>();
            new_col->vector_->reserve(n_monte_carlos_);
        }
    }
}

MonteCarloResultsModel::~MonteCarloResultsModel()
{
}

void MonteCarloResultsModel::ProcessMonteCarloResult(
    std::shared_ptr<FitResults> results,
    const std::vector<std::string>& sample_names,
    const std::vector<std::string>& parameter_names,
    const std::vector<SampleConcentrations>& concentrations)
{
    assert(!sample_names.empty());
    unsigned i = name_lookup_.at(sample_names.front());
    for (const auto& col_type : available_column_types_)
        monte_carlo_data_.at(i).at(col_type)->vector_->
                emplace_back(GetElement(*results, col_type).toDouble());
    exit_flags_.at(i)->emplace_back(results->exit_flag);
}

Qt::ItemFlags MonteCarloResultsModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable |
           Qt::ItemIsEditable |
           Qt::ItemIsEnabled;
}

int MonteCarloResultsModel::columnCount(const QModelIndex& parent) const
{
    return column_types_.size();
}

QVariant MonteCarloResultsModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return ResultsModel::headerData(section, orientation, role);

    try
    {
        return GetVariantColumnName(column_types_.at(section));
    }
    catch (...)
    {
    }
    return QVariant();
}

QVariant MonteCarloResultsModel::data(const QModelIndex& index, int role) const
{
    if (unsigned(index.row()) >= monte_carlo_data_.size() || !n_monte_carlos_)
        return QVariant();
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            assert(original_fit_results_ &&
                   original_fit_results_->size() == monte_carlo_data_.size());
            HistogramData data = boost::apply_visitor(
                    GetOrCreateDataVisitor(*this, index),
                    column_types_.at(index.column()));
            return QVariant::fromValue(data);
        }
        case Qt::UserRole:
        {
            QStringList titles = boost::apply_visitor(
                    GetAxisTitlesVisitor(*this),
                    column_types_.at(index.column()));
            titles.push_front(sample_names_.at(index.row()));
            return titles;
        }
        default:;
    }
    return QVariant();
}

bool MonteCarloResultsModel::setData(
        const QModelIndex& index,
        const QVariant& value,
        int role)
{
    return true;
}

void MonteCarloResultsModel::SetOriginalResults(
        std::shared_ptr<const StandardFitResultsModel::ResultsVector> results)
{
    original_fit_results_ = results;
    if (original_fit_results_need_to_be_added_to_plot_data_vectors_)
        AddOriginalFitResultsToPlotDataVectors();
}

QString MonteCarloResultsModel::GetVariantColumnName(
        const ColumnTypeVariant& type) const
{
    return boost::apply_visitor(GetHeaderNameVisitor(*this), type);
}

void MonteCarloResultsModel::ExportMonteCarloData(
        MonteCarloExporter& exporter,
        QList<ExtendedColumnType> col_types) const
{
    auto new_end = std::remove_if(
            col_types.begin(),
            col_types.end(),
            [](const ExtendedColumnType& c)
            {
                return (c.first == ColumnType::DEGREES_OF_FREEDOM);
            });
    col_types.erase(new_end, col_types.end());

    exporter << "Name";
    for (auto col : col_types)
        exporter << GetVariantColumnName(col);
    exporter.NextSample();
    assert(masks_.size() == monte_carlo_data_.size());
    for (unsigned i = 0; i < monte_carlo_data_.size(); ++i)
    {
        const auto& sample = monte_carlo_data_[i];
        for (unsigned j = 0; j < n_monte_carlos_; ++j)
        {
            if (!masks_.at(i)->IsEnabled(j)) continue;
            exporter << sample_names_.at(i);
            for (auto col : col_types)
            {
                if (col.first == ColumnType::CONVERGENCE)
                    exporter << QString::fromStdString(
                            FitResults::GetExitFlagAsString(
                                exit_flags_.at(i)->at(j)));
                else
                    exporter << (*sample.at(col)->vector_)[j];
            }
            exporter.NextSample();
        }
    }
}

unsigned MonteCarloResultsModel::NumberOfMonteCarlos() const
{
    return n_monte_carlos_;
}

const QList<ColumnTypeVariant>& MonteCarloResultsModel::GetColumnTypes() const
{
    return column_types_;
}

const QList<ExtendedColumnType>&
MonteCarloResultsModel::GetAvailableColumnTypes() const
{
    return available_column_types_;
}


unsigned int MonteCarloResultsModel::NumberOfBins() const
{
    return n_bins_;
}

void MonteCarloResultsModel::SetColumnTypes(
        const QList<ColumnTypeVariant>& column_types)
{
    emit layoutAboutToBeChanged();
    column_types_ = column_types;
    RecreateDataToIndexMapAndConnectSignals();
    emit layoutChanged();
}

void MonteCarloResultsModel::SetNumberOfBins(int n_bins)
{
    if (!rowCount() || !columnCount())
        return;
    n_bins_ = n_bins;
}

const QList<ColumnTypeVariant> MonteCarloResultsModel::PrepareColumnTypes(
        unsigned int n_parameters, unsigned int n_gases)
{
    const unsigned n_covariances = (n_parameters * (n_parameters - 1) / 2);
    QList<ColumnTypeVariant> cols;

    for (unsigned i = 0; i < n_covariances; ++i)
    {
        auto indices = GetCovarianceMatrixIndexes(i);
        cols.push_back(ColumnType2D(
                {ColumnType::PARAMETER_ESTIMATE, indices.first},
                {ColumnType::PARAMETER_ESTIMATE, indices.second}));
    }

    for (unsigned i = 0; i < n_parameters; ++i)
        cols.push_back(ExtendedColumnType({ColumnType::PARAMETER_ESTIMATE, i}));

    return cols;
}

void MonteCarloResultsModel::RecreateDataToIndexMapAndConnectSignals()
{
    for (auto& object : data_to_index_map_)
        disconnect(object.first, 0, this, 0);
    data_to_index_map_.clear();
    assert(plot_data_1d_.size() == plot_data_2d_.size());
    for (unsigned i = 0; i < plot_data_1d_.size(); ++i)
    {
        const auto& map_1d = plot_data_1d_[i];
        const auto& map_2d = plot_data_2d_[i];
        for (const auto& plot_data : map_1d)
        {
            int column = column_types_.indexOf(plot_data.first);
            if (column >= 0)
                AddToDataToIndexMapAndConnectSignals(
                    plot_data.second.get(), i, column);
        }
        for (const auto& plot_data : map_2d)
        {
            int column = column_types_.indexOf(plot_data.first);
            if (column >= 0)
                AddToDataToIndexMapAndConnectSignals(
                    plot_data.second.get(), i, column);
        }
    }
}

void MonteCarloResultsModel::AddToDataToIndexMapAndConnectSignals(
        HistogramDataBase* data, int line, int column) const
{
    data_to_index_map_[data] = createIndex(line, column);
    connect(data, SIGNAL(HistogramChanged()), this, SLOT(UpdatePlot()));
    connect(data, SIGNAL(SelectionChanged()), this, SLOT(UpdatePlot()));
}

void MonteCarloResultsModel::AddOriginalFitResultsToPlotDataVectors()
{
    for (unsigned i = 0; i < plot_data_1d_.size(); ++i)
    {
        auto& map = plot_data_1d_[i];
        for (auto& plot_data : map)
            plot_data.second->SetOriginalResult(GetElement(
                    *original_fit_results_->at(i), plot_data.first).toDouble());
    }
    for (unsigned i = 0; i < plot_data_2d_.size(); ++i)
    {
        auto& map = plot_data_2d_[i];
        for (auto& plot_data : map)
        {
            plot_data.second->SetOriginalResult(
                GetElement(*original_fit_results_->at(i),
                           plot_data.first.first).toDouble(),
                GetElement(*original_fit_results_->at(i),
                           plot_data.first.second).toDouble());
        }
    }
}

void MonteCarloResultsModel::UpdatePlot()
{
    QModelIndex index = data_to_index_map_.at(sender());
    emit dataChanged(index, index);
    emit AnyDataChanged();
}
