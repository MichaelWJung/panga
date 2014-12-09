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


#include <qwt_color_map.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

#include <cassert>
#include <cmath>

#include "colormap.h"
#include "contourplot.h"
#include "contourplotdata.h"

class ContourPlotZoomer : public QwtPlotZoomer
{

public:
    ContourPlotZoomer(int xAxis, int yAxis, QWidget* canvas, bool doReplot = true) :
            QwtPlotZoomer(xAxis, yAxis, canvas,doReplot)
    {
        // Mausbelegungen vertauschen, sodass Rechtsklick eine Stufe rauszoomt
        // und Mittelklick ganz raus.
        QVector<MousePattern>& mouse_patterns = mousePattern();
        assert(mouse_patterns.size() >= 3);
        std::swap(mouse_patterns[1], mouse_patterns[2]);
        setMousePattern(mouse_patterns);
    }

protected:
    QSizeF minZoomSize() const
    {
        return QwtPlotZoomer::minZoomSize() / 10e20;
    }
};

ContourPlot::ContourPlot(QWidget* parent) :
    QwtPlot(parent),
    spectrogram_(new QwtPlotSpectrogram("χ²")),
    colorbar_(nullptr),
    zoomer_(nullptr),
    picker_(nullptr)
{
    spectrogram_->setDisplayMode(QwtPlotSpectrogram::ContourMode, false);
    spectrogram_->setDisplayMode(QwtPlotSpectrogram::ImageMode  , true );
    spectrogram_->setColorMap(new LogarithmicColorMap());
    spectrogram_->attach(this);
    
    // A color bar on the right axis
    colorbar_ = axisWidget(QwtPlot::yRight);
    colorbar_->setTitle("χ²");
    colorbar_->setColorBarEnabled(true);
    colorbar_->setColorMap(QwtInterval(1, 100),
                           new LogarithmicColorMap());
    colorbar_->setColorBarWidth(30);

    setAxisScale(QwtPlot::yRight, 1, 100);
    setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine());
    enableAxis(QwtPlot::yRight);
    axisScaleDraw(QwtPlot::yRight)->setMinimumExtent(70);
    
    zoomer_ = new ContourPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas());
    zoomer_->setStateMachine(new QwtPickerDragRectMachine());
    zoomer_->setRubberBandPen(QColor(Qt::darkGray));
    zoomer_->setZoomBase();
    
    picker_ = new QwtPlotPicker(canvas());
    picker_->setStateMachine(new QwtPickerTrackerMachine());
    picker_->setRubberBand(QwtPicker::CrossRubberBand);
    picker_->setRubberBandPen(QColor(Qt::darkGray));

    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);
    axisScaleEngine(QwtPlot::yLeft  )->setAttribute(QwtScaleEngine::Floating);
    axisScaleEngine(QwtPlot::yRight )->setAttribute(QwtScaleEngine::Floating);
    
    connect(picker_, SIGNAL(moved(QPointF)),
            this, SLOT(EmitCursorPosition(QPointF)));
    connect(picker_, SIGNAL(activated(bool)),
            this, SLOT(EmitCursorNotOverPlot(bool)));

    replot();
}

ContourPlot::~ContourPlot()
{
}

void ContourPlot::SetRasterData(ContourPlotData* data)
{
    spectrogram_->setData(data);
    connect(data, SIGNAL(ReplotNeeded()), this, SLOT(replot()));
    connect(data, SIGNAL(ZIntervalChanged()), this, SLOT(replot()));
    connect(data, SIGNAL(ZIntervalChanged()), this, SLOT(UpdateColorBar()));
    connect(zoomer_, SIGNAL(zoomed(QRectF)),
            data, SLOT(AutoUpdateWithNextInitRaster()));
    replot();
}

void ContourPlot::SetAxesScales(QwtInterval x_interval, QwtInterval y_interval)
{
    setAxisScale(QwtPlot::xBottom,
                 x_interval.minValue(),
                 x_interval.maxValue());
    setAxisScale(QwtPlot::yLeft,
                 y_interval.minValue(),
                 y_interval.maxValue());
    zoomer_->setZoomBase();
}

void ContourPlot::SetAxesTitles(QString x_title, QString y_title)
{
    setAxisTitle(QwtPlot::xBottom, x_title);
    setAxisTitle(QwtPlot::yLeft  , y_title);
}

void ContourPlot::SetTitle(QString title)
{
    setTitle(title);
}

void ContourPlot::UpdateColorBar()
{
    QwtInterval chi2_interval = spectrogram_->data()->interval(Qt::ZAxis);
    colorbar_->setColorMap(chi2_interval, new LogarithmicColorMap());
    setAxisScale(QwtPlot::yRight,
                 chi2_interval.minValue(),
                 chi2_interval.maxValue());
    updateAxes();
}

void ContourPlot::EmitCursorNotOverPlot(bool over_plot)
{
    if (!over_plot)
        emit CursorPositionChanged(false, QPointF());
}

void ContourPlot::EmitCursorPosition(const QPointF& position)
{
    emit CursorPositionChanged(true, position);
}

