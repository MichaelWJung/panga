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
#include <cmath>
#include <utility>

#include "core/fitting/noblefitfunction.h"
#include "core/misc/defines.h"

#include "contourplotdata.h"

const int ContourPlotFitter::BLOCKSIZE_REDUCTION_FACTOR = 2;

ContourPlotFitter::ContourPlotFitter(ContourPlotData* data) :
    QThread(),
    data_(data),
    concentrations_(),
    fit_configurations_(),
    results_(),
    chi2_values_(32, 32),
    sample_number_(-1),
    x_parameter_(-1),
    y_parameter_(-1),
    n_parameters_(0),
    new_jobs_(),
    completed_jobs_(),
    plot_stack_(),
    plot_stack_infos_(),
    fitted_parameter_names_(),
    all_jobs_prepared_(false),
    new_jobs_mutex_(),
    completed_jobs_mutex_(),
    plot_stack_mutex_(),
    new_job_condition_(),
    new_future_condition_(),
    interrupted_(false)
{
}

ContourPlotFitter::~ContourPlotFitter()
{
}

void ContourPlotFitter::PrepareForFit()
{
    interrupted_ = false;
}

void ContourPlotFitter::SetConcentrations(const RunData& concentrations)
{
    concentrations_.clear();
    for (auto it = concentrations.concentrations_begin();
         it != concentrations.concentrations_end();
         ++it)
        concentrations_.push_back({*it});
}

void ContourPlotFitter::SetFitConfigurations(
        const std::vector< FitConfiguration >& fit_configurations)
{
    fit_configurations_ = fit_configurations;
    
    assert(!fit_configurations_.empty());
    n_parameters_ = fit_configurations_[0].fit_parameter_config.size();
}

void ContourPlotFitter::SetSampleNumber(int sample)
{
    sample_number_ = sample;
    InterruptFitAndInvalidateResults();
}

void ContourPlotFitter::SetAxesParameters(int x_param, int y_param)
{
    if (unsigned(x_param) >= n_parameters_ ||
        unsigned(y_param) >= n_parameters_ ||
        x_param == y_param)
        throw std::invalid_argument("Parameter indices equal or too high");
    
    x_parameter_ = x_param;
    y_parameter_ = y_param;
    InterruptFitAndInvalidateResults();
}

bool ContourPlotFitter::SetFixedParameters(
        const std::vector<std::pair<int, double>>& fixed_parameters)
{
    std::vector<std::pair<int, double>> new_fixed_parameters(fixed_parameters);
    std::vector<std::pair<int, double>> current_fixed_parameters(
            fixed_parameters_);
    
    fixed_parameters_ = fixed_parameters;

    auto cmp = [&](const std::pair<int, double>& p)
               {
                   return p.first == x_parameter_ ||
                          p.first == y_parameter_;
               };   
    new_fixed_parameters.erase(
            std::remove_if(new_fixed_parameters.begin(),
                           new_fixed_parameters.end(),
                           cmp),
            new_fixed_parameters.end());
    current_fixed_parameters.erase(
            std::remove_if(current_fixed_parameters.begin(),
                           current_fixed_parameters.end(),
                           cmp),
            current_fixed_parameters.end());
    
    if (new_fixed_parameters != current_fixed_parameters)
    {
        InterruptFitAndInvalidateResults();
        return true;
    }
    return false;
}

void ContourPlotFitter::SetFitArea(const QRectF& rect, const QSize& raster)
{
    new_rect_ = rect;
    new_raster_ = raster;
    InterruptFitAndInvalidateResults();
}

void ContourPlotFitter::run()
{
    if (sample_number_ < 0 ||
        unsigned(sample_number_) >= fit_configurations_.size() ||
        !new_rect_.isValid() ||
        !new_raster_.isValid())
        return;
        
    PrepareVariables();
    
    FitConfiguration config(fit_configurations_.at(sample_number_));
    assert(config.sample_numbers.size() == 1);
    const std::vector<SampleConcentrations>& concentrations(
            concentrations_.at(sample_number_));
    std::shared_ptr<NobleFitFunction> function(
            std::make_shared<NobleFitFunction>(
                config.model,
                *config.GetParameterMap(),
                concentrations));

    RemoveFixedParametersFromFit(config);
    DetermineFittedParameterNames(config);
    std::vector<std::pair<int, double>> parameters = PrepareParametersVector();
        
    LevenbergMarquardtFitter fitter(function);
   
    unsigned n_cpus = boost::thread::hardware_concurrency();
    if (n_cpus == 0) n_cpus = 1;
    boost::thread_group worker_threads;
    for (unsigned i = 0; i < n_cpus; ++i)
        worker_threads.create_thread(
                std::bind(&ContourPlotFitter::DoFittingJobs,
                          this,
                          parameters,
                          std::cref(fitter),
                          std::cref(config)));
    CreateJobs();
    
    if (!ProcessResults())
        worker_threads.interrupt_all();
    worker_threads.join_all();
    
}

void ContourPlotFitter::GetResultsPointers(
        const QRectF*& rect,
        const QSize*& raster,
        const Eigen::MatrixXd*& chi2_values,
        const std::vector<std::vector<std::shared_ptr<const FitResults>>>*&
            results,
        const std::vector<QString>*& parameter_names)
{
    boost::lock_guard<boost::mutex> lock(plot_stack_mutex_);
    if (plot_stack_.empty())
    {
        rect = nullptr;
        raster = nullptr;
        chi2_values = nullptr;
        results = nullptr;
        parameter_names = nullptr;
    }
    else
    {
        rect = &plot_stack_.back().rect;
        raster = &plot_stack_.back().raster;
        chi2_values = &plot_stack_.back().chi2_values;
        results = &plot_stack_.back().results;
        parameter_names = &fitted_parameter_names_;
    }
}

void ContourPlotFitter::InterruptFitAndInvalidateResults()
{
    emit ResultsInvalidated();
    interrupted_ = true;
}

void ContourPlotFitter::PrepareVariables()
{
    all_jobs_prepared_ = false;
    new_jobs_.clear();
    completed_jobs_.clear();
    plot_stack_infos_.clear();
    {
        boost::lock_guard<boost::mutex> lock(plot_stack_mutex_);
        if (!plot_stack_.empty())
        {
            emit ResultsInvalidated();
            plot_stack_.clear();
        }
    }
    
    data_rect_= new_rect_;
    data_raster_ = new_raster_;
    
    chi2_values_.resize(data_raster_.width(), data_raster_.height());
    chi2_values_.fill(std::numeric_limits<double>::quiet_NaN());
    results_.clear();
    results_.resize(data_raster_.width());
    for (auto& r : results_)
        r.resize(data_raster_.height());
}

void ContourPlotFitter::CreateJobs()
{
    const int width = data_raster_.width();
    const int height = data_raster_.height();
    int smaller_border = std::min(width, height);
    int blocksize = std::pow(BLOCKSIZE_REDUCTION_FACTOR,
                             std::ceil(std::log(smaller_border) /
                                       std::log(BLOCKSIZE_REDUCTION_FACTOR)));
    // Am Anfang sehr groß gewählt, sodass im ersten Schritt auch wirklich
    // keine Punkte übersprungen werden.
    int old_blocksize = blocksize * 10000;
    int n_fits = 0;
    std::vector<JobParameters> working_vector_;
    working_vector_.reserve(width * height);
    while (blocksize > 0)
    {
        working_vector_.clear();
        const int beginning = blocksize == 1 ? 0 : std::ceil(blocksize / 2.);
        const int old_beginning = std::ceil(old_blocksize / 2.);
        for (int i = beginning; i < width; i += blocksize)
            for (int j = beginning; j < height; j += blocksize)
                // Nur hinzufügen wenn nicht schon bei der nächsthöheren
                // Blocksize geschehen.
                if ((i - old_beginning % old_blocksize) ||
                    (j - old_beginning % old_blocksize))
                {
                    working_vector_.push_back(JobParameters(i, j));
                    ++n_fits;
                }
        if (n_fits)        
            plot_stack_infos_.push_back(PlotStackInfo(n_fits, blocksize));
        old_blocksize = blocksize;
        blocksize /= BLOCKSIZE_REDUCTION_FACTOR;
        
        {
            boost::lock_guard<boost::mutex> lock(new_jobs_mutex_);
            std::move(working_vector_.begin(),
                      working_vector_.end(),
                      std::back_inserter(new_jobs_));
        }
        new_job_condition_.notify_all();
    }
    
    all_jobs_prepared_ = true;
}

void ContourPlotFitter::RemoveFixedParametersFromFit(FitConfiguration& config) const
{
    std::vector<int> parameters_to_remove;
    parameters_to_remove.push_back(x_parameter_);
    parameters_to_remove.push_back(y_parameter_);
    for (auto it = fixed_parameters_.cbegin();
         it != fixed_parameters_.cend();
         ++it)
         parameters_to_remove.push_back(it->first);
    std::sort(parameters_to_remove.begin(),
              parameters_to_remove.end(),
              std::greater<int>());
    parameters_to_remove.erase(std::unique(parameters_to_remove.begin(),
                                           parameters_to_remove.end()),
                               parameters_to_remove.end());
    std::for_each(parameters_to_remove.begin(),
                  parameters_to_remove.end(),
                  std::bind(&FitParameterConfig::RemoveParameter,
                            &config.fit_parameter_config,
                            std::placeholders::_1));
}

void ContourPlotFitter::DetermineFittedParameterNames(FitConfiguration& config)
{
    boost::lock_guard<boost::mutex> lock(plot_stack_mutex_);
    if (!fitted_parameter_names_.empty())
    {
        emit ResultsInvalidated();
        fitted_parameter_names_.clear();
    }
    for (const auto& name : config.fit_parameter_config.names())
        fitted_parameter_names_.push_back(QString::fromStdString(name));
}

std::vector<std::pair<int, double>>
ContourPlotFitter::PrepareParametersVector() const
{
    std::vector<std::pair<int, double>> parameters;
    parameters.push_back(std::make_pair(x_parameter_, 0.));
    parameters.push_back(std::make_pair(y_parameter_, 0.));
    std::copy_if(fixed_parameters_.cbegin(),
                 fixed_parameters_.cend(),
                 std::back_inserter(parameters),
                 [&](const std::pair<int, double>& p)
                 {
                     return p.first != x_parameter_ &&
                            p.first != y_parameter_;
                 });
    return parameters;
}

bool ContourPlotFitter::ResultsLeft() const
{
    return !completed_jobs_.empty() ||
           !new_jobs_.empty() ||
           !all_jobs_prepared_;
}

ContourPlotFitter::JobPromise ContourPlotFitter::GetNewJob()
{
    JobParameters params;
    {
        boost::unique_lock<boost::mutex> lock(new_jobs_mutex_);
        if (new_jobs_.empty() && all_jobs_prepared_) throw NoJobsLeft();
        
        while (new_jobs_.empty())
            new_job_condition_.wait(lock);
        
        params = new_jobs_.front();
        new_jobs_.pop_front();
    }
    
    boost::promise<std::shared_ptr<FitResults>> promise;
    boost::unique_future<std::shared_ptr<FitResults>> future(
            promise.get_future());
    
    JobFuture job_future;
    job_future.params = params;
    job_future.future = std::move(future);
    {
        boost::lock_guard<boost::mutex> lock(completed_jobs_mutex_);
        completed_jobs_.push_back(std::move(job_future));
    }
    
    new_future_condition_.notify_one();
            
    JobPromise job_promise;
    job_promise.params = params;
    job_promise.promise = std::move(promise);
    return job_promise;
}

ContourPlotFitter::JobResults ContourPlotFitter::GetNextResults()
{
    JobFuture future;
    {
        boost::unique_lock<boost::mutex> lock(completed_jobs_mutex_);
        
        if (!ResultsLeft())
            throw NoResultsLeft();
        
        while(completed_jobs_.empty())
            new_future_condition_.wait(lock);
        
        future = std::move(completed_jobs_.front());
        completed_jobs_.pop_front();
    }
    
    JobResults results;
    results.params = future.params;
    results.results = future.future.get();
    return results;
}

noble_align_function void ContourPlotFitter::DoFittingJobs(
        std::vector<std::pair<int, double>> parameters,
        const LevenbergMarquardtFitter& fitter,
        const FitConfiguration& config)
{
    std::shared_ptr<LevenbergMarquardtFitter> local_fitter(fitter.clone());
    std::shared_ptr<NobleFitFunction> function(
            std::dynamic_pointer_cast<NobleFitFunction>(
                    local_fitter->GetFitFunction()));
    
    try
    {
        while (true)
        {
            ContourPlotFitter::JobPromise job_promise = GetNewJob();
            const JobParameters& params = job_promise.params;
            
            parameters[0].second = data_rect_.left() +
                    params.i * data_rect_.width()  / data_raster_.width();
            parameters[1].second = data_rect_.top() +
                    params.j * data_rect_.height() / data_raster_.height();

            function->FixParameters(parameters);

            std::shared_ptr<FitResults> results =
                    local_fitter->fit(std::make_shared<FitParameterConfig>(
                        config.fit_parameter_config));
                       
            job_promise.promise.set_value(results);
            
            boost::this_thread::interruption_point();
        }
    }
    catch (NoJobsLeft)
    {
    }
}

bool ContourPlotFitter::ProcessResults()
{
    QwtInterval chi2_interval;
    
    auto it = plot_stack_infos_.cbegin();
    int n_fits = 0;
    try
    {
        while (true)
        {
            if (interrupted_)
                return false;
            JobResults next_results(GetNextResults());
            const JobParameters& params = next_results.params;
            std::shared_ptr<FitResults> results = next_results.results;
            
            chi2_values_(params.i, params.j) = results->chi_square;
            results_.at(params.i).at(params.j) = results;
            
            ++n_fits;
            if (std::isfinite(results->chi_square))
            {
                if (!chi2_interval.isValid())
                    chi2_interval.setInterval(results->chi_square,
                                              results->chi_square);
                else
                    chi2_interval |= results->chi_square;
            }
            
            assert(it != plot_stack_infos_.cend());
            if (n_fits == it->n_fits)
            {
                UpdatePlotStack(it->blocksize);
                if (chi2_interval.isValid())
                    emit NewDataAvailable(chi2_interval.minValue(),
                                          chi2_interval.maxValue());
                else
                    emit NewDataAvailable(0., 0.);
                ++it;
            }
        }
    }
    catch (NoResultsLeft)
    {
    }
    
    return true;
}

void ContourPlotFitter::UpdatePlotStack(int blocksize)
{
    const int width = data_raster_.width();
    const int height = data_raster_.height();
    const int x_size = width / blocksize;
    const int y_size = height / blocksize;
    const int initial_index = blocksize == 1 ? 0 : std::ceil(blocksize / 2.);
    Eigen::MatrixXd matrix(x_size, y_size);
    std::vector<std::vector<std::shared_ptr<const FitResults>>> results(
        x_size, std::vector<std::shared_ptr<const FitResults>>(y_size));
    
    for (int x = 0; x < x_size; ++x)
        for (int y = 0; y < y_size; ++y)
        {
            const int i = initial_index + x * blocksize;
            const int j = initial_index + y * blocksize;
            matrix(x, y) = chi2_values_(i, j);
            results.at(x).at(y) = results_.at(i).at(j);
        }

    const double rect_width = x_size * blocksize * data_rect_.width()  / width;
    const double rect_height= y_size * blocksize * data_rect_.height() / height;
    QSize raster(x_size, y_size);
    QRectF rect(data_rect_.left(), data_rect_.top(), rect_width, rect_height);
    
    {
        boost::lock_guard<boost::mutex> lock(plot_stack_mutex_);
        plot_stack_.emplace_back();
        plot_stack_.back().rect = rect;
        plot_stack_.back().raster = raster;
        plot_stack_.back().chi2_values = std::move(matrix);
        plot_stack_.back().results = std::move(results);
    }
}
