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


#include <qwt_matrix_raster_data.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "core/fitting/fitresults.h"
#include "core/fitting/levenbergmarquardtfitter.h"
#include "core/fitting/noblefitfunction.h"

#include "contourplotdata.h"

ContourPlotData::ContourPlotData() :
    QObject(),
    QwtRasterData(),
    fitter_(this),
    plot_rect_(0., 0., 10., 10.),
    plot_raster_(32, 32),
    pointers_mutex_(),
    pointers_valid_(false),
    data_rect_(nullptr),
    data_raster_(nullptr),
    chi2_values_(nullptr),
    results_(nullptr),
    fitted_parameter_names_(nullptr),
    auto_update_chi2_interval_(true),
    current_chi2_interval_(),
    chi2_data_interval_(),
    auto_update_(false),
    auto_update_with_next_init_raster_(false)
{
    double max = 1e50;
    double min = -max;
    setInterval(Qt::XAxis, QwtInterval(min, max));
    setInterval(Qt::YAxis, QwtInterval(min, max));
    setInterval(Qt::ZAxis, QwtInterval(1, 2));
    
    connect(&fitter_, SIGNAL(NewDataAvailable(double, double)),
            this, SLOT(UpdateResultsPointers()));
    connect(&fitter_, SIGNAL(NewDataAvailable(double, double)),
            this, SLOT(UpdateDataInterval(double, double)));
    connect(&fitter_, SIGNAL(NewDataAvailable(double,double)),
            this, SLOT(EmitReplotNeeded()));
    connect(&fitter_, SIGNAL(ResultsInvalidated()),
            this, SLOT(InvalidatePointers()), Qt::DirectConnection);
}

ContourPlotData::~ContourPlotData()
{
    InterruptFitAndWait();
}

void ContourPlotData::Fit()
{
    InterruptFitAndWait();
    fitter_.SetFitArea(plot_rect_, plot_raster_);
    fitter_.PrepareForFit();
    fitter_.start();
}

void ContourPlotData::InterruptFitAndWait()
{
    if (fitter_.isRunning())
    {
        fitter_.InterruptFitAndInvalidateResults();
        fitter_.wait();
    }
}

void ContourPlotData::SetConcentrations(const RunData& concentrations)
{
    fitter_.SetConcentrations(concentrations);
}

void ContourPlotData::SetFitConfigurations(
        const std::vector<FitConfiguration>& fit_configurations)
{
    fitter_.SetFitConfigurations(fit_configurations);
}

QString ContourPlotData::GetInfoForPoint(double x, double y) const
{
    boost::lock_guard<boost::mutex> lock(pointers_mutex_);
    if (!pointers_valid_ || !data_rect_->contains(x, y))
        return QString();
    std::pair<int, int> indices = GetIndicesForCoordinates(x, y);
    std::shared_ptr<const FitResults> results(results_->at(indices.first)
                                                       .at(indices.second));
    
    assert(fitted_parameter_names_->size() ==
            unsigned(results->best_estimate.size()));
    
    QString string;
    for (unsigned i = 0; i < fitted_parameter_names_->size(); ++i)
        string += (*fitted_parameter_names_)[i] + "="
                + QString::number(results->best_estimate[i]) + ", ";
    if (string.size() > 0)
        string.remove(string.size() - 3, 2);
    return string;
}

double ContourPlotData::value(double x, double y) const
{
    boost::lock_guard<boost::mutex> lock(pointers_mutex_);
    if (!pointers_valid_ || !data_rect_->contains(x, y))
        return std::numeric_limits<double>::quiet_NaN();
    std::pair<int, int> indices = GetIndicesForCoordinates(x, y);
    return (*chi2_values_)(indices.first, indices.second);
}

void ContourPlotData::initRaster(const QRectF& rect, const QSize& raster)
{
    plot_rect_ = rect;
    plot_raster_ = raster;
    if (auto_update_with_next_init_raster_)
    {
        UpdatePlotIfAutoUpdateEnabled();
        auto_update_with_next_init_raster_ = false;
    }
}

void ContourPlotData::discardRaster()
{
}

bool ContourPlotData::AutoUpdateChi2Interval() const
{
    return auto_update_chi2_interval_;
}

void ContourPlotData::AutoUpdateWithNextInitRaster()
{
    auto_update_with_next_init_raster_ = true;
}

void ContourPlotData::SetSampleNumber(int sample)
{
    fitter_.SetSampleNumber(sample);
    UpdatePlotIfAutoUpdateEnabled();
}

void ContourPlotData::SetAxesParameters(int x_param, int y_param)
{
    fitter_.SetAxesParameters(x_param, y_param);
    UpdatePlotIfAutoUpdateEnabled();
}

void ContourPlotData::SetFixedParameters(
        std::vector<std::pair<int, double>> fixed_parameters)
{
    if (fitter_.SetFixedParameters(fixed_parameters))
        UpdatePlotIfAutoUpdateEnabled();
}

void ContourPlotData::SetAutoUpdatePlot(bool enabled)
{
    auto_update_ = enabled;
    UpdatePlotIfAutoUpdateEnabled();
}

void ContourPlotData::SetAutoUpdateChi2Interval(bool enabled)
{
    auto_update_chi2_interval_ = enabled;
    UpdatePlotChi2IntervalIfAutoUpdateEnabled();
}

bool ContourPlotData::SetCustomChi2Interval(QwtInterval interval)
{
    assert(interval.minValue() > 0 && interval.maxValue() > 0);
    if (auto_update_chi2_interval_)
        return false;
    if (current_chi2_interval_ != interval)
    {
        current_chi2_interval_ = interval;
        setInterval(Qt::ZAxis, current_chi2_interval_);
        emit ZIntervalChanged();
    }
    return true;
}

void ContourPlotData::EmitReplotNeeded()
{
    emit ReplotNeeded();
}

void ContourPlotData::UpdatePlotIfAutoUpdateEnabled()
{
    if (auto_update_) Fit();
}

void ContourPlotData::UpdateResultsPointers()
{
    boost::lock_guard<boost::mutex> lock(pointers_mutex_);
    fitter_.GetResultsPointers(data_rect_,
                               data_raster_,
                               chi2_values_,
                               results_,
                               fitted_parameter_names_);

    if (!data_rect_   ||
        !data_raster_ ||
        !chi2_values_ ||
        !results_     ||
        !fitted_parameter_names_)
        pointers_valid_ = false;
    else
        pointers_valid_ = true;
}

void ContourPlotData::UpdateDataInterval(
        double chi2_min, double chi2_max)
{
    chi2_data_interval_ = QwtInterval(chi2_min, chi2_max);
    UpdatePlotChi2IntervalIfAutoUpdateEnabled();
}

void ContourPlotData::UpdatePlotChi2IntervalIfAutoUpdateEnabled()
{
    if (auto_update_chi2_interval_)
    {
        current_chi2_interval_ = chi2_data_interval_;
        setInterval(Qt::ZAxis, chi2_data_interval_);
        emit ZIntervalChanged();
    }
}

void ContourPlotData::InvalidatePointers()
{
    boost::lock_guard<boost::mutex> lock(pointers_mutex_);
    pointers_valid_ = false;
}

std::pair<int, int> ContourPlotData::GetIndicesForCoordinates(double x,
                                                              double y) const
{
    int i = (x - data_rect_->left()) /
            data_rect_->width () * data_raster_->width ();
    int j = (y - data_rect_->top ()) /
            data_rect_->height() * data_raster_->height();

    if (i >= data_raster_->width())
        i = data_raster_->width() - 1;

    if (j >= data_raster_->height())
        j = data_raster_->height() - 1;
    
    return std::make_pair(i, j);
}
