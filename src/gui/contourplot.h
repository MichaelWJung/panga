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


#ifndef CONTOURPLOT_H
#define CONTOURPLOT_H

#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>

class ContourPlotData;
class ContourPlotZoomer;
class QwtPlotPicker;
class QwtScaleWidget;

class ContourPlot : public QwtPlot
{
    Q_OBJECT
    
public:
    explicit ContourPlot(QWidget* parent = 0);
    virtual ~ContourPlot();
    void SetRasterData(ContourPlotData* data);
    
public slots:
    void SetAxesScales(QwtInterval x_interval, QwtInterval y_interval);
    void SetAxesTitles(QString x_title, QString y_title);
    void SetTitle(QString title);
    
signals:
    void CursorPositionChanged(bool over_plot, QPointF position);
    
private slots:
    void UpdateColorBar();
    void EmitCursorNotOverPlot(bool over_plot);
    void EmitCursorPosition(const QPointF& position);
    
private:
    QwtPlotSpectrogram* spectrogram_;
    QwtScaleWidget* colorbar_;
    ContourPlotZoomer* zoomer_;
    QwtPlotPicker* picker_;
};

#endif // CONTOURPLOT_H
