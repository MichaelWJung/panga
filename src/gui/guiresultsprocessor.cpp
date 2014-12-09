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


#include <QCoreApplication>

#include <boost/make_shared.hpp>

#include <algorithm>

#include "commons.h"
#include "montecarloresultsmodel.h"
#include "standardfitresultsmodel.h"

#include "guiresultsprocessor.h"

GuiResultsProcessor::GuiResultsProcessor(
        const std::vector<std::string>& sample_names,
        const std::vector<ModelParameter>& parameters,
        const std::set<GasType>& gases_in_use,
        unsigned n_monte_carlos) :
    results_model_(nullptr),
    monte_carlo_results_model_(nullptr),
    samples_processed_(0U),
    was_interrupted_(false)
{
    std::vector<QString> q_sample_names(sample_names.size());
    std::transform(sample_names.begin(),
                   sample_names.end(),
                   q_sample_names.begin(),
                   StdStringToQString());
    results_model_ = new StandardFitResultsModel(
            q_sample_names,
            parameters,
            gases_in_use);
    monte_carlo_results_model_ = new MonteCarloResultsModel(
            q_sample_names,
            parameters,
            gases_in_use,
            n_monte_carlos);
}

GuiResultsProcessor::~GuiResultsProcessor()
{
    if (!static_cast<QObject*>(results_model_)->parent())
        delete results_model_;
    if (!static_cast<QObject*>(monte_carlo_results_model_)->parent())
        delete monte_carlo_results_model_;
}

void GuiResultsProcessor::ProcessResult(
        std::shared_ptr<FitResults> results,
        const std::vector<std::string>& sample_names,
        const std::vector<std::string>& parameter_names,
        const std::vector<SampleConcentrations>& concentrations)
{
    results_model_->ProcessResult(boost::make_shared<FitResults>(*results),
                                  sample_names,
                                  parameter_names, concentrations);
}

void GuiResultsProcessor::ProcessMonteCarloResult(
        std::shared_ptr<FitResults> results,
        const std::vector<std::string>& sample_names,
        const std::vector<std::string>& parameter_names,
        const std::vector<SampleConcentrations>& concentrations)
{
    monte_carlo_results_model_->ProcessMonteCarloResult(
            results, sample_names, parameter_names, concentrations);

    QCoreApplication::processEvents();
    if (was_interrupted_) throw InterruptedByUser();
    emit MonteCarloResultProcessed(++samples_processed_);
}

QAbstractTableModel* GuiResultsProcessor::GetResultsModel() const
{
    return results_model_;
}

QAbstractTableModel* GuiResultsProcessor::GetMonteCarloResultsModel() const
{
    return monte_carlo_results_model_;
}

void GuiResultsProcessor::Interrupt()
{
    was_interrupted_ = true;
}
