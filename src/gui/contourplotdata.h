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


#ifndef CONTOURPLOTDATA_H
#define CONTOURPLOTDATA_H

#include <qwt_raster_data.h>

#include <boost/thread.hpp>

#include "core/fitting/fitconfiguration.h"
#include "core/fitting/fitresults.h"
#include "core/misc/rundata.h"

#include "contourplotfitter.h"

class LevenbergMarquardtFitter;
class QwtMatrixRasterData;

class ContourPlotData : public QObject, public QwtRasterData
{
    Q_OBJECT
    
public:
    explicit ContourPlotData();
    virtual ~ContourPlotData();
    
    void Fit();
    
    void SetConcentrations(const RunData& concentrations);
    void SetFitConfigurations(
            const std::vector<FitConfiguration>& fit_configurations);
    
    QString GetInfoForPoint(double x, double y) const;
    
    // QwtRasterData-Methoden
    virtual double value(double x, double y) const;
    
    void initRaster(const QRectF& rect, const QSize& raster);
    void discardRaster();
    bool AutoUpdateChi2Interval() const;
    
public slots:
    void AutoUpdateWithNextInitRaster();
    void SetSampleNumber(int sample);
    void SetAxesParameters(int x_param, int y_param);
    void SetFixedParameters(
            std::vector<std::pair<int, double>> fixed_parameters);
    void SetAutoUpdatePlot(bool enabled);
    void SetAutoUpdateChi2Interval(bool enabled);
    
    //! Setzt eigenes Intervall, aber nur wenn Auto-Update deaktiviert ist.
    /*!
     * \return Gibt an ob das Setzen erfolgreich war.
     */
    bool SetCustomChi2Interval(QwtInterval interval);
    
signals:
    void ReplotNeeded();
    void ZIntervalChanged();
    
private slots:
    void EmitReplotNeeded();
    void UpdatePlotIfAutoUpdateEnabled();
    void UpdateResultsPointers();
    void UpdateDataInterval(double chi2_min, double chi2_max);
    void UpdatePlotChi2IntervalIfAutoUpdateEnabled();
    void InvalidatePointers();

private:
    void InterruptFitAndWait();
    
    std::pair<int, int> GetIndicesForCoordinates(double x, double y) const;
    ContourPlotFitter fitter_;
    
    QRectF plot_rect_;
    QSize plot_raster_;
    
    mutable boost::mutex pointers_mutex_;
    bool pointers_valid_;
    const QRectF* data_rect_;
    const QSize* data_raster_;
    const Eigen::MatrixXd* chi2_values_;
    const std::vector<std::vector<std::shared_ptr<const FitResults>>>* results_;
    const std::vector<QString>* fitted_parameter_names_;
    
    bool auto_update_chi2_interval_;
    QwtInterval current_chi2_interval_;
    QwtInterval chi2_data_interval_;
    
    bool auto_update_;
    bool auto_update_with_next_init_raster_;
};

#endif // CONTOURPLOTDATA_H
