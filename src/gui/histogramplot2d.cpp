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

#include <qwt_color_map.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_picker_machine.h>
#include <qwt_picker.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

#include <algorithm>
#include <cassert>
#include <cmath>

#include "colormap.h"
#include "histogramdata2d.h"
#include "histogramplotzoomer.h"
#include "matrixrasterdataproxy.h"

#include "histogramplot2d.h"

class Cross : public QwtPlotItem
{
public:
    explicit Cross(QPen pen, double size) :
        QwtPlotItem(),
        pen_(pen),
        size_(size)
    {
    }

    void SetPosition(QPointF position)
    {
        x_value_ = position.x();
        y_value_ = position.y();
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
        if (std::isnan(x_value_) || std::isnan(y_value_)) return;

        QLineF line1(
                x_map.transform(x_value_), y_map.transform(y_value_) - size_,
                x_map.transform(x_value_), y_map.transform(y_value_) + size_);
        QLineF line2(
                x_map.transform(x_value_) - size_, y_map.transform(y_value_),
                x_map.transform(x_value_) + size_, y_map.transform(y_value_));

        painter->setPen(pen_);
        painter->drawLine(line1);
        painter->drawLine(line2);
    }

private:
    QPen pen_;
    double size_;
    double x_value_;
    double y_value_;
};

class ShadedPolygonBackground : public QwtPlotItem
{
public:
    ShadedPolygonBackground(QPen pen, QBrush brush) :
        brush_(brush),
        pen_(pen),
        inverted_(false)
    {
        setZ(0.0);
    }

    void SetPolygon(const QPolygonF& polygon)
    {
        polygon_ = polygon;
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
        if (polygon_.empty())
            return;
        QPolygonF polygon(polygon_);
        for (QPointF& p : polygon)
        {
            p.setX(x_map.transform(p.x()));
            p.setY(y_map.transform(p.y()));
        }
        QPainterPath path;
        if (!inverted_)
        {
            const double off = 100;
            path.addRect(-off, -off,
                         canvas_rect.width()  + 2 * off,
                         canvas_rect.height() + 2 * off);
        }
        path.addPolygon(polygon);

        painter->setPen(pen_);
        painter->setBrush(brush_);
        painter->drawPath(path);
    }

private:
    QBrush brush_;
    QPen pen_;
    QPolygonF polygon_;
    bool inverted_;
};

HistogramPlot2D::HistogramPlot2D(bool colorbar_enabled,
                                 bool in_table,
                                 QWidget* parent) :
    HistogramPlot(in_table, parent),
    colorbar_enabled_(colorbar_enabled),
    shade_(new ShadedPolygonBackground(QPen(Qt::darkRed),
                                QColor(64, 0, 0, 64))),
    spectrogram_(new QwtPlotSpectrogram()),
    zoomer_(new HistogramPlotZoomer(canvas())),
    picker_(new QwtPlotPicker(canvas())),
    original_fit_result_(new Cross(QPen(QBrush(Qt::darkGreen), 2), 15.))
{
    spectrogram_->setZ(0.);
    spectrogram_->attach(this);
    spectrogram_->setColorMap(new HistogramColorMap());

    original_fit_result_->setZ(1.);
    original_fit_result_->attach(this);

    shade_->setZ(2.);
    shade_->attach(this);

    picker_->setStateMachine(new QwtPickerPolygonMachine());
    picker_->setRubberBand(QwtPicker::PolygonRubberBand);
    zoomer_->setStateMachine(new QwtPickerDragRectMachine());
    zoomer_->setRubberBand(QwtPicker::RectRubberBand);

    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);
    axisScaleEngine(QwtPlot::yLeft  )->setAttribute(QwtScaleEngine::Floating);
    axisScaleEngine(QwtPlot::yRight )->setAttribute(QwtScaleEngine::Floating);

    // Die vorderen 3 MousePatterns entfernen, damit die hinteren, die
    // zusätlich eine gedrückte Umschalt-Taste verlangen an ihre Stelle treten.
    QVector<QwtEventPattern::MousePattern>& mouse_patterns =
            picker_->mousePattern();
    assert(mouse_patterns.size() >= 6);
    for(unsigned i = 0; i < 3; ++i)
        mouse_patterns.pop_front();
    picker_->setMousePattern(mouse_patterns);

    connect(picker_, SIGNAL(selected(QPolygon)),
            this, SLOT(Select(QPolygon)));

    if (colorbar_enabled_)
        EnableColorBar();
}

HistogramPlot2D::~HistogramPlot2D()
{
}

void HistogramPlot2D::SetData(HistogramData data)
{
    if (data_)
        disconnect(zoomer_, 0, data_.get(), 0);
    data_ = boost::get<SharedHistogramData2D>(data);

    connect(zoomer_, SIGNAL(selected(QRectF)),
            data_.get(), SLOT(ZoomIn(QRectF)));
    connect(zoomer_, SIGNAL(ZoomOut()),
            data_.get(), SLOT(ZoomOut()));

    HistogramPlot::SetData(data);

    UpdatePlot();
}

void HistogramPlot2D::UpdatePlot()
{
    if (!data_->size) return;

    try
    {
        QwtMatrixRasterData* rasterdata =
                data_->GetHistogramRasterData();
        MatrixRasterDataProxy* rasterdata_proxy(
                new MatrixRasterDataProxy(rasterdata));
        rasterdata_proxy->setInterval(Qt::XAxis, data_->GetXInterval());
        rasterdata_proxy->setInterval(Qt::YAxis, data_->GetYInterval());
        rasterdata_proxy->setInterval(
                Qt::ZAxis, rasterdata->interval(Qt::ZAxis));

        spectrogram_->setData(rasterdata_proxy);

        if (colorbar_enabled_)
        {
            axisWidget(QwtPlot::yRight)->setColorMap(
                    spectrogram_->data()->interval(Qt::ZAxis),
                    new HistogramColorMap());
            setAxisScale(QwtPlot::yRight,
                        spectrogram_->data()->interval(Qt::ZAxis).minValue(),
                        spectrogram_->data()->interval(Qt::ZAxis).maxValue());
        }

        shade_->SetPolygon(data_->GetSelectionPolygon());
        shade_->SetInverted(data_->IsSelectionInverted());

        original_fit_result_->SetPosition(data_->GetOriginalResult());

        HistogramPlot::UpdatePlot();

        if (information_label_)
        {
            QString text = "Correlation: " +
                           QString::number(data_->CalcCorrelation());
            information_label_->setText(information_label_->text() + "\n\n" +
                                        text);
        }
    }
    catch (...)
    {
    }

    replot();
}

bool HistogramPlot2D::SelectionActive()
{
    return !data_->GetSelectionPolygon().isEmpty();
}

void HistogramPlot2D::Select(QPolygon polygon)
{
    if (polygon.size() <= 2)
        return;
    QPolygonF transformed_polygon(polygon);
    for (QPointF& p : transformed_polygon)
    {
        p.setX(invTransform(QwtPlot::xBottom, p.x()));
        p.setY(invTransform(QwtPlot::yLeft  , p.y()));
    }
    transformed_polygon.push_back(transformed_polygon.front());
    data_->SetSelectionPolygon(transformed_polygon);
}

void HistogramPlot2D::EnableColorBar()
{
    QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Counts");
    rightAxis->setColorBarEnabled(true);
    rightAxis->setColorBarWidth(30);
    enableAxis(QwtPlot::yRight);
}
