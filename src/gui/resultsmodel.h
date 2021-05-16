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


#ifndef RESULTSMODEL_H
#define RESULTSMODEL_H

#include <QAbstractTableModel>
#include <QString>

#include <set>

#include "core/fitting/fitresultsprocessor.h"
#include "core/models/modelparameter.h"

enum class ColumnType
{
    DEGREES_OF_FREEDOM,
    CHI_SQUARE,
    PROBABILITY,
    PARAMETER_ESTIMATE,
    PARAMETER_ESTIMATE_ERROR,
    CONVERGENCE,
    CORRELATION,
    RESIDUAL,
    EQUILIBRIUM_CONCENTRATION,
    EQUILIBRIUM_CONCENTRATION_ERROR,
    MODEL_CONCENTRATION,
    MODEL_CONCENTRATION_ERROR,
    MEASURED_CONCENTRATION,
    MEASURED_CONCENTRATION_ERROR,
    CHI,
    DELTA_NEON,
    DELTA_NEON_ERROR,
    RAD_HE,
    RAD_HE_ERROR,
    RAD_HE3,
    RAD_HE3_ERROR,
};

typedef std::pair<ColumnType, unsigned> ExtendedColumnType;

class ResultsModel :  public QAbstractTableModel
{
    Q_OBJECT

public:
    ResultsModel(const std::vector<QString>& sample_names,
                 const std::vector<ModelParameter>& parameters,
                 const std::set<GasType>& gases_in_use,
                 QObject* parent = 0);
    virtual ~ResultsModel();

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    QString GetColumnName(const ExtendedColumnType& type) const;

    const QList<ExtendedColumnType> GetAvailableColumnTypes() const;
    const QList<ExtendedColumnType> GetDoubleColumnTypes() const;

protected:
    QVariant GetElement(const FitResults& results,
                        const ExtendedColumnType& column_type) const;
    double GetResidualForGas(const FitResults& results,
                             unsigned gas_index) const;
    double GetDeltaNeon(const FitResults& results) const;
    double GetDeltaNeonError(const FitResults& results) const;             
    GasType GetGasFromIndex(unsigned i) const;

    //! \brief Durchläuft die Elemente der Oberen Dreiecksmatrix spaltenweise
    //! bis der übergebene Index erreicht ist.
    static std::pair<unsigned, unsigned>
    GetCovarianceMatrixIndexes(unsigned n);

    const std::vector<QString> sample_names_;
    const std::vector<ModelParameter> parameters_;
    const std::set<GasType> gases_in_use_;

    const unsigned n_parameters_;
};

#endif // RESULTSMODEL_H