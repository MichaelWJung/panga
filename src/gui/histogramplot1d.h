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


#ifndef HISTOGRAMPLOT1D_H
#define HISTOGRAMPLOT1D_H

#include "histogramplot.h"

class HistogramPlotZoomer;
class ShadedBackground;
class QwtPlotHistogram;
class QwtPlotPicker;
class VerticalLine;

class HistogramPlot1D : public HistogramPlot
{
    Q_OBJECT
    
public:
    explicit HistogramPlot1D(bool in_table, QWidget* parent = 0);
    ~HistogramPlot1D();
    
    void SetData(HistogramData data);
    
    bool SelectionActive();
    
public slots:
    void UpdatePlot();

private slots:
    void Select(QRectF rect);

private:
    SharedHistogramData1D data_;
    ShadedBackground* shade_;
    QwtPlotHistogram* histogram_;
    HistogramPlotZoomer* zoomer_;
    QwtPlotPicker* picker_;
    VerticalLine* original_fit_result_;
};

#endif // HISTOGRAMPLOT1D_H
