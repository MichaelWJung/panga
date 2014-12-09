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


#ifndef HISTOGRAMPLOT2D_H
#define HISTOGRAMPLOT2D_H

#include "histogramplot.h"

#include "histogramdata.h"

class Cross;
class HistogramPlotZoomer;
class ShadedPolygonBackground;
class QwtPicker;
class QwtPlotSpectrogram;

class HistogramPlot2D : public HistogramPlot
{
    Q_OBJECT
    
public:
    explicit HistogramPlot2D(bool colorbar_enabled,
                             bool in_table,
                             QWidget* parent = nullptr);
    virtual ~HistogramPlot2D();

    void SetData(HistogramData data);
    void UpdatePlot();
    
    bool SelectionActive();
    
private slots:
    void Select(QPolygon polygon);
    
private:
    bool colorbar_enabled_;
    void EnableColorBar();
    SharedHistogramData2D data_;
    ShadedPolygonBackground* shade_;
    QwtPlotSpectrogram* spectrogram_;
    HistogramPlotZoomer* zoomer_;
    QwtPicker* picker_;
    Cross* original_fit_result_;
};

#endif // HISTOGRAMPLOT2D_H
