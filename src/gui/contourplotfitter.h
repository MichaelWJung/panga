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


#ifndef CONTOURPLOTFITTER_H
#define CONTOURPLOTFITTER_H

#include <QRectF>
#include <QSize>
#include <QThread>

#include <boost/thread.hpp>

#include <deque>

#include "core/fitting/fitconfiguration.h"
#include "core/fitting/fitresults.h"
#include "core/fitting/levenbergmarquardtfitter.h"
#include "core/misc/rundata.h"

class ContourPlotData;

class ContourPlotFitter : public QThread
{
    Q_OBJECT
    
public:
    ContourPlotFitter(ContourPlotData* data);
    virtual ~ContourPlotFitter();
    
    void PrepareForFit();
    
    void SetConcentrations(const RunData& concentrations);
    void SetFitConfigurations(
            const std::vector<FitConfiguration>& fit_configurations);
    
    void SetSampleNumber(int sample);
    void SetAxesParameters(int x_param, int y_param);
    bool SetFixedParameters(
            const std::vector<std::pair<int, double>>& fixed_parameters);
    void SetFitArea(const QRectF& rect, const QSize& raster);
    
    void GetResultsPointers(
            const QRectF*& rect,
            const QSize*& raster,
            const Eigen::MatrixXd*& chi2_values,
            const std::vector<std::vector<std::shared_ptr<const FitResults>>>*&
                results,
            const std::vector<QString>*& parameter_names);
                            
    void InterruptFitAndInvalidateResults();
                            
signals:
    void NewDataAvailable(double chi2_min, double chi2_max);
    void ResultsInvalidated();
    
protected:
    virtual void run();
    
private:    
    class NoJobsLeft {};
    class NoResultsLeft {};
    
    struct JobParameters
    {
        JobParameters() : i(-1), j(-1) {}
        JobParameters(int i, int j) : i(i), j(j) {}
        int i;
        int j;
    };
    
    struct JobPromise
    {
        JobParameters params;
        boost::promise<std::shared_ptr<FitResults>> promise;
    };
    
    struct JobResults
    {
        JobParameters params;
        std::shared_ptr<FitResults> results;
    };
    
    struct JobFuture
    {
        JobParameters params;
        boost::unique_future<std::shared_ptr<FitResults>> future;
    };
    
    struct Plot
    {
        QRectF rect;
        QSize raster;
        Eigen::MatrixXd chi2_values;
        std::vector<std::vector<std::shared_ptr<const FitResults>>> results;
    };
    
    struct PlotStackInfo
    {
        PlotStackInfo(int n_fits, int blocksize) :
                n_fits(n_fits), blocksize(blocksize) {}
        int n_fits;
        int blocksize;
        QSize size;
    };
    
    void PrepareVariables();
    void CreateJobs();
    void RemoveFixedParametersFromFit(FitConfiguration& config) const;
    void DetermineFittedParameterNames(FitConfiguration& config);
    std::vector<std::pair<int, double>> PrepareParametersVector() const;
    bool ResultsLeft() const;
    JobPromise GetNewJob();
    JobResults GetNextResults();
    void DoFittingJobs(std::vector<std::pair<int, double>> parameters,
                       const LevenbergMarquardtFitter& fitter,
                       const FitConfiguration& config);
    /*!
     * \return Wahr wenn alle Fits abgearbeitet wurden.
     *   Falsch falls unterbrochen.
     */
    bool ProcessResults();
    void UpdatePlotStack(int blocksize);
    
    ContourPlotData* data_;
    
    std::vector<std::vector<SampleConcentrations>> concentrations_;
    std::vector<FitConfiguration> fit_configurations_;
        
    std::vector<std::vector<std::shared_ptr<const FitResults>>> results_;
    Eigen::MatrixXd chi2_values_;

    int sample_number_;
    int x_parameter_;
    int y_parameter_;
    unsigned n_parameters_;
    std::vector<std::pair<int, double>> fixed_parameters_;
    
    QRectF new_rect_;
    QSize new_raster_;
    QRectF data_rect_;
    QSize data_raster_;
    
    std::deque<ContourPlotFitter::JobParameters> new_jobs_;
    std::deque<ContourPlotFitter::JobFuture> completed_jobs_;
    
    std::vector<Plot> plot_stack_;
    std::vector<PlotStackInfo> plot_stack_infos_;
    std::vector<QString> fitted_parameter_names_;
    
    bool all_jobs_prepared_;
    boost::mutex new_jobs_mutex_;
    boost::mutex completed_jobs_mutex_;
    boost::mutex plot_stack_mutex_;
    boost::condition_variable new_job_condition_;
    boost::condition_variable new_future_condition_;
    bool interrupted_;
    
    static const int BLOCKSIZE_REDUCTION_FACTOR;
};

#endif // CONTOURPLOTFITTER_H
