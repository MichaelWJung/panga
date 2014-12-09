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


#ifndef GUIRESULTSPROCESSOR_H
#define GUIRESULTSPROCESSOR_H

#include <QProgressDialog>

#include <set>
#include <string>
#include <vector>

#include "core/fitting/fitresultsprocessor.h"
#include "core/models/modelparameter.h"

class MonteCarloResultsModel;
class StandardFitResultsModel;
class QAbstractTableModel;
class QObject;

class GuiResultsProcessor : public QObject, public FitResultsProcessor
{
    Q_OBJECT
    
public:
    //!Konstruktor.
    /*!
     * Der Parameter parent wird nur an die erzeugten Modelle weitergeleitet.
     * Diese Klasse hier muss immer selbständig gelöscht werden.
     */ 
    GuiResultsProcessor(const std::vector<std::string>& sample_names,
                        const std::vector<ModelParameter>& parameters,
                        const std::set<GasType>& gases_in_use,
                        unsigned n_monte_carlos);
    virtual ~GuiResultsProcessor();
    
    void ProcessResult(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& parameter_names,
            const std::vector<SampleConcentrations>& concentrations);
    
    void ProcessMonteCarloResult(
            std::shared_ptr<FitResults> results,
            const std::vector<std::string>& sample_names,
            const std::vector<std::string>& parameter_names,
            const std::vector<SampleConcentrations>& concentrations);
    
    QAbstractTableModel* GetResultsModel() const;
    QAbstractTableModel* GetMonteCarloResultsModel() const;

    class InterruptedByUser {};
    
public slots:
    void Interrupt();
               
signals:
    void MonteCarloResultProcessed(int i);
    
private:
    StandardFitResultsModel* results_model_;
    MonteCarloResultsModel* monte_carlo_results_model_;
    unsigned samples_processed_;
    bool was_interrupted_;
};

#endif // GUIRESULTSPROCESSOR_H
