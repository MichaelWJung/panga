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


#ifndef MONTECARLORESULTSMODEL_H
#define MONTECARLORESULTSMODEL_H

#include <QList>
#include <QMetaType>
#include <QStringList>

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/make_shared.hpp>
#include <boost/variant.hpp>

#include "core/fitting/fitresults.h"
#include "core/models/modelparameter.h"
#include "core/misc/typedefs.h"

#include "datavector.h"
#include "histogramdata.h"
#include "serializationhelpers.h"
#include "standardfitresultsmodel.h"

#include "resultsmodel.h"

class MonteCarloResultsModel;
namespace boost { namespace serialization {
template<class Archive>
void save_construct_data(
        Archive& ar, const MonteCarloResultsModel* t, const unsigned version);
}}

class MonteCarloExporter;
typedef std::pair<ExtendedColumnType, ExtendedColumnType> ColumnType2D;
typedef boost::variant<ExtendedColumnType, ColumnType2D> ColumnTypeVariant;

Q_DECLARE_METATYPE(ColumnTypeVariant)

//! Modellklasse, die die Monte-Carlo-Daten verwaltet.
/*!
 * \warning Bevor diese Klasse in einem View verwendet wird müssen auch diese
 * ursprünglichen Fitergebnisse mit SetOriginalResults übergeben werden.
 */
class MonteCarloResultsModel : public ResultsModel
{
    Q_OBJECT

public:
    MonteCarloResultsModel(
            const std::vector<QString>& sample_names,
            const std::vector<ModelParameter>& parameters,
            const std::set<GasType>& gases_in_use,
            unsigned n_monte_carlos,
            QObject* parent = 0);

    virtual ~MonteCarloResultsModel();

    void ProcessMonteCarloResult(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& parameter_names,
            const std::vector<SampleConcentrations>& concentrations);

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex& index,
                          int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole);

    void SetOriginalResults(std::shared_ptr<
            const StandardFitResultsModel::ResultsVector> results);

    QString GetVariantColumnName(const ColumnTypeVariant& type) const;
    void ExportMonteCarloData(MonteCarloExporter& exporter,
                              QList<ExtendedColumnType> col_types) const;
    unsigned NumberOfMonteCarlos() const;
    const QList<ColumnTypeVariant>& GetColumnTypes() const;
    const QList<ExtendedColumnType>& GetAvailableColumnTypes() const;
    unsigned NumberOfBins() const;

public slots:
    void SetColumnTypes(const QList<ColumnTypeVariant>& column_types);
    void SetNumberOfBins(int n_bins);

signals:
    void AnyDataChanged();

private:
    typedef std::map<ExtendedColumnType, boost::shared_ptr<DataVector>>
            SingleSampleMonteCarloData;

    static const QList<ColumnTypeVariant> PrepareColumnTypes(
            unsigned n_parameters, unsigned n_gases);

    void RecreateDataToIndexMapAndConnectSignals();
    void AddToDataToIndexMapAndConnectSignals(
            HistogramDataBase* data, int line, int column) const;

    //! \brief Wird benötigt um auch bei älteren Save-Files die Fitergebnisse
    //! in den MC-Plots anzuzeigen.
    void AddOriginalFitResultsToPlotDataVectors();

private slots:
    void UpdatePlot();

private:

    QList<ColumnTypeVariant> column_types_;
    QList<ExtendedColumnType> available_column_types_;
    const unsigned n_monte_carlos_;
    SharedBinNumber n_bins_;

    std::shared_ptr<const StandardFitResultsModel::ResultsVector>
            original_fit_results_;
    std::vector<SingleSampleMonteCarloData> monte_carlo_data_;
    const std::map<std::string, unsigned> name_lookup_;

    // Wird lazy aus monte_carlo_data_ u.a. erzeugt, daher mutable.
    mutable std::vector<std::map<ExtendedColumnType,
                                 SharedHistogramData1D>> plot_data_1d_;
    mutable std::vector<std::map<ColumnType2D,
                                 SharedHistogramData2D>> plot_data_2d_;
    std::vector<SharedMask> masks_;
    std::vector<boost::shared_ptr<std::vector<Eigen::LM::Status>>> exit_flags_;

    mutable std::map<QObject*, QModelIndex> data_to_index_map_;

    //! \brief Wird benötigt um auch bei älteren Save-Files die Fitergebnisse
    //! in den MC-Plots anzuzeigen.
    bool original_fit_results_need_to_be_added_to_plot_data_vectors_;


    class GetHeaderNameVisitor : public boost::static_visitor<QString>
    {
    public:
        explicit GetHeaderNameVisitor(const MonteCarloResultsModel& model);
        QString operator()(const ExtendedColumnType& type) const;
        QString operator()(const ColumnType2D& type) const;

    private:
        const MonteCarloResultsModel& model_;
    };

    class GetAxisTitlesVisitor : public boost::static_visitor<QStringList>
    {
    public:
        explicit GetAxisTitlesVisitor(const MonteCarloResultsModel& model);
        QStringList operator()(const ExtendedColumnType& type)const;
        QStringList operator()(const ColumnType2D& type) const;

    private:
        const MonteCarloResultsModel& model_;
    };

    class GetOrCreateDataVisitor : public boost::static_visitor<HistogramData>
    {
    public:
        GetOrCreateDataVisitor(const MonteCarloResultsModel& model,
                               const QModelIndex& index);
        HistogramData operator()(const ExtendedColumnType& type) const;
        HistogramData operator()(const ColumnType2D& type) const;
        void CreatePlotData(const ExtendedColumnType& type) const;
        void CreatePlotData(const ColumnType2D& type) const;

    private:
        const MonteCarloResultsModel& model_;
        const QModelIndex& index_;
    };

    friend class boost::serialization::access;
    template<class Archive>
    friend void boost::serialization::save_construct_data(
            Archive& ar,
            const MonteCarloResultsModel* t,
            const unsigned version);

    template<class Archive>
    void save(Archive& ar, const unsigned version) const
    {
        std::list<ColumnTypeVariant> column_types = column_types_.toStdList();
        std::list<ExtendedColumnType> available_column_types =
                available_column_types_.toStdList();
        ar << column_types
           << available_column_types
           << n_bins_
           << monte_carlo_data_
           << plot_data_1d_
           << plot_data_2d_
           << masks_
           << exit_flags_;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned version)
    {
        std::list<ColumnTypeVariant> column_types;
        std::list<ExtendedColumnType> available_column_types;
        ar >> column_types
           >> available_column_types;
        column_types_ = QList<ColumnTypeVariant>::fromStdList(column_types);
        available_column_types_ =
                QList<ExtendedColumnType>::fromStdList(available_column_types);
        ar >> n_bins_;

        if (version < 3)
        {
            std::vector<std::map<ExtendedColumnType,
                    boost::shared_ptr<std::vector<double>>>> old_mc_data;
            ar >> old_mc_data;
            monte_carlo_data_.clear();
            monte_carlo_data_.resize(old_mc_data.size());
            for (unsigned i = 0; i < old_mc_data.size(); ++i)
                for (auto it = old_mc_data[i].begin();
                     it != old_mc_data[i].end();
                     ++it)
                {
                    monte_carlo_data_[i][it->first] =
                            boost::make_shared<DataVector>(it->second);
                }
        }
        else
            ar >> monte_carlo_data_;

        if (version < 2)
        {
            std::map<std::string, unsigned> dummy;
            ar >> dummy;
        }

        ar >> plot_data_1d_
           >> plot_data_2d_
           >> masks_
           >> exit_flags_;

        if (version < 1)
            original_fit_results_need_to_be_added_to_plot_data_vectors_ = true;

        RecreateDataToIndexMapAndConnectSignals();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(MonteCarloResultsModel, 3)

namespace boost { namespace serialization {
template<class Archive>
inline void save_construct_data(
        Archive& ar, const MonteCarloResultsModel* t, const unsigned version)
{
    ar << t->sample_names_
       << t->parameters_
       << t->gases_in_use_
       << t->n_monte_carlos_;
}

template<class Archive>
inline void load_construct_data(
        Archive& ar, MonteCarloResultsModel* t, const unsigned version)
{
    std::vector<QString> sample_names;
    std::vector<ModelParameter> parameters;
    std::set<GasType> gases_in_use;
    unsigned n_monte_carlos_;
    ar >> sample_names
       >> parameters
       >> gases_in_use
       >> n_monte_carlos_;

       ::new(t) MonteCarloResultsModel(sample_names, parameters,
                                       gases_in_use, n_monte_carlos_);
}

}}

#endif // MONTECARLORESULTSMODEL_H
