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


#include <QLabel>
#include <QPainter>

#include <qwt_picker_machine.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_engine.h>

#include <algorithm>
#include <cassert>
#include <cmath>

#include "histogramdata1d.h"
#include "histogramplotzoomer.h"
#include "intervalseriesdataproxy.h"

#include "histogramplot1d.h"

class VerticalLine : public QwtPlotItem
{
public:
    explicit VerticalLine(QPen pen) :
        QwtPlotItem(),
        pen_(pen)
    {
    }

    void SetValue(double value)
    {
        value_ = value;
    }

    virtual int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    virtual void draw(
            QPainter* painter,
            const QwtScaleMap& x_map,
            const QwtScaleMap& y_map,
            const QRectF& canvas_rect ) const
    {
        if (std::isnan(value_)) return;

        const double off = qMax(pen_.widthF(), 1.0);

        QLineF line(x_map.transform(value_), canvas_rect.top()    - off,
                    x_map.transform(value_), canvas_rect.bottom() + off);

        painter->setPen(pen_);
        painter->drawLine(line);
    }

private:
    QPen pen_;
    double value_;
};

class ShadedBackground : public QwtPlotItem
{
public:
    ShadedBackground(QPen pen, QBrush brush) :
        brush_(brush),
        pen_(pen)
    {
        setZ(0.0);
    }

    void SetInterval(const QwtInterval& interval)
    {
        interval_ = interval;
    }

    void SetInverted(bool inverted)
    {
        inverted_ = inverted;
    }

    virtual int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    virtual void draw(
            QPainter* painter,
            const QwtScaleMap& x_map,
            const QwtScaleMap& y_map,
            const QRectF& canvas_rect ) const
    {
        if (!interval_.isValid())
            return;

        painter->setPen(pen_);
        painter->setBrush(brush_);
        const double off = qMax(pen_.widthF(), 1.0);

        if (inverted_)
        {
            QRectF rect;
            rect.setTop   (canvas_rect.top()    - off);
            rect.setBottom(canvas_rect.bottom() + off);
            rect.setLeft  (x_map.transform(interval_.minValue()));
            rect.setRight (x_map.transform(interval_.maxValue()));

            rect = rect.normalized();

            painter->drawRect(rect);
        }
        else
        {
            QRectF left_rect;
            left_rect.setTop   (canvas_rect.top()    - off);
            left_rect.setBottom(canvas_rect.bottom() + off);
            left_rect.setLeft  (canvas_rect.left()   - off);
            left_rect.setRight (x_map.transform(interval_.minValue()));

            left_rect = left_rect.normalized();

            QRectF right_rect;
            right_rect.setTop   (canvas_rect.top()    - off);
            right_rect.setBottom(canvas_rect.bottom() + off);
            right_rect.setLeft  (x_map.transform(interval_.maxValue()));
            right_rect.setRight (canvas_rect.right()  + off);

            right_rect = right_rect.normalized();

            painter->drawRect(left_rect);
            painter->drawRect(right_rect);
        }
    }

private:
    QBrush brush_;
    QPen pen_;
    QwtInterval interval_;
    bool inverted_;
};

HistogramPlot1D::HistogramPlot1D(bool in_table, QWidget* parent) :
    HistogramPlot(in_table, parent),
    shade_(new ShadedBackground(QPen(Qt::darkRed),
                                QColor(64, 0, 0, 64))),
    histogram_(new QwtPlotHistogram()),
    zoomer_(new HistogramPlotZoomer(canvas())),
    picker_(new QwtPlotPicker(canvas())),
    original_fit_result_(new VerticalLine(QPen(Qt::darkGreen)))
{
    histogram_->setZ(0.);
    histogram_->setPen(QPen(Qt::black));
    histogram_->setStyle(QwtPlotHistogram::Outline);
    histogram_->attach(this);

    original_fit_result_->setZ(1.);
    original_fit_result_->attach(this);

    shade_->setZ(2.);
    shade_->attach(this);

    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);

    zoomer_->setStateMachine(new QwtPickerDragRectMachine());
    zoomer_->setRubberBand(QwtPicker::RectRubberBand);
    picker_->setStateMachine(new QwtPickerDragRectMachine());
    picker_->setRubberBand(QwtPicker::RectRubberBand);

    // Die vorderen 3 MousePatterns entfernen, damit die hinteren, die
    // zusätlich eine gedrückte Umschalt-Taste verlangen an ihre Stelle treten.
    QVector<QwtEventPattern::MousePattern>& mouse_patterns =
            picker_->mousePattern();
    assert(mouse_patterns.size() >= 6);
    for(unsigned i = 0; i < 3; ++i)
        mouse_patterns.pop_front();
    picker_->setMousePattern(mouse_patterns);

    connect(picker_, SIGNAL(selected(QRectF)),
            this, SLOT(Select(QRectF)));
}

HistogramPlot1D::~HistogramPlot1D()
{
}

void HistogramPlot1D::SetData(HistogramData data)
{
    if (data_)
        disconnect(zoomer_, 0, data_.get(), 0);
    data_ = boost::get<SharedHistogramData1D>(data);

    connect(zoomer_, SIGNAL(selected(QRectF)),
            data_.get(), SLOT(ZoomIn(QRectF)));
    connect(zoomer_, SIGNAL(ZoomOut()),
            data_.get(), SLOT(ZoomOut()));

    HistogramPlot::SetData(data);

    UpdatePlot();
}

void HistogramPlot1D::UpdatePlot()
{
    if (!data_->size) return;

    try
    {
        IntervalSeriesDataProxy* seriesdata(
                new IntervalSeriesDataProxy(data_->GetHistogramData()));
        histogram_->setData(seriesdata);

        shade_->SetInterval(data_->GetSelectionInterval());
        shade_->SetInverted(data_->IsSelectionInverted());

        original_fit_result_->SetValue(data_->GetOriginalResult());

        HistogramPlot::UpdatePlot();

        if (information_label_)
        {
            QString text = "Mean: " + QString::number(data_->CalcMean()) + "\n";
            text += "Stddev: " + QString::number(data_->CalcStdDev());
            information_label_->setText(information_label_->text() + "\n\n" +
                                        text);
        }
    }
    catch (...)
    {
    }

    replot();
}

bool HistogramPlot1D::SelectionActive()
{
    return data_->GetSelectionInterval().isValid();
}

void HistogramPlot1D::Select(QRectF rect)
{
    data_->SetSelectionInterval(QwtInterval(rect.left(), rect.right()));
}
