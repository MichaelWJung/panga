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


#ifndef STANDARDFITRESULTSMODEL_H
#define STANDARDFITRESULTSMODEL_H

#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>

#include <QList>

#include <algorithm>
#include <memory>

#include "serializationhelpers.h"

#include "resultsmodel.h"

class RunData;
class StandardFitResultsModel;

namespace boost { namespace serialization {
template<class Archive>
void save_construct_data(
        Archive& ar, const StandardFitResultsModel* t, const unsigned version);
}}

class StandardFitResultsModel : public ResultsModel
{
    Q_OBJECT

public:
    StandardFitResultsModel(const std::vector<QString>& sample_names,
                            const std::vector<ModelParameter>& parameters,
                            const std::set<GasType>& gases_in_use,
                            QObject* parent = 0);
    virtual ~StandardFitResultsModel();

    void ProcessResult(
            boost::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& parameter_names,
            const std::vector<SampleConcentrations>& concentrations);

    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    void AddUnusedConcentrations(const RunData& run_data);

    typedef std::vector<boost::shared_ptr<FitResults>> ResultsVector;
    std::shared_ptr<const ResultsVector> GetResultsVector() const;
    bool DoesAnySampleNeedMonteCarlo() const;

private:
    bool IsParameterInNormalRange(const FitResults& results,
                                  const ExtendedColumnType& column_type) const;
    bool DoesSampleNeedMonteCarlo(unsigned sample_number) const;

    friend class boost::serialization::access;
    template<class Archive>
    friend void boost::serialization::save_construct_data(
            Archive& ar,
            const StandardFitResultsModel* t,
            const unsigned version);

    template<class Archive>
    void save(Archive& ar, const unsigned version) const
    {
        ar << *results_;
        std::list<ExtendedColumnType> column_types = column_types_.toStdList();
        ar << column_types;
    }

    template<class Archive>
    void load(Archive& ar, const unsigned version)
    {
        ResultsVector results;
        ar >> results;
        results_ = std::make_shared<ResultsVector>(std::move(results));

        std::list<ExtendedColumnType> column_types;
        ar >> column_types;
        //! \todo Vielleicht hierfür besser einen eigenen Konstruktor anlegen,
        //! um sich const_cast und save/load_construct_data zu sparen.
        const_cast<QList<ExtendedColumnType>&>(column_types_) =
                QList<ExtendedColumnType>::fromStdList(column_types);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()


    std::shared_ptr<ResultsVector> results_;

    const QList<ExtendedColumnType> column_types_;

    mutable bool sample_needs_monte_carlo_initialized_;
    mutable std::vector<bool> sample_needs_monte_carlo_;
};

namespace boost { namespace serialization {
template<class Archive>
inline void save_construct_data(
        Archive& ar, const StandardFitResultsModel* t, const unsigned version)
{
    ar << t->sample_names_
       << t->parameters_
       << t->gases_in_use_;
}

template<class Archive>
inline void load_construct_data(
        Archive& ar, StandardFitResultsModel* t, const unsigned version)
{
    std::vector<QString> sample_names;
    std::vector<ModelParameter> parameters;
    std::set<GasType> gases_in_use;
    ar >> sample_names
       >> parameters
       >> gases_in_use;

    ::new(t) StandardFitResultsModel(sample_names, parameters, gases_in_use);
}

}}

#endif // STANDARDFITRESULTSMODEL_H
