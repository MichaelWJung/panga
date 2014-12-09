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


#include "histogramdata2d.h"


HistogramData2D::HistogramData2D(
        unsigned long size,
        const SharedBinNumber& n_bins_default,
        SharedMask mask,
        boost::shared_ptr<const DataVector> x_data,
        boost::shared_ptr<const DataVector> y_data) :
    HistogramDataBase(size, n_bins_default, mask),
    x_data_(x_data),
    y_data_(y_data),
    original_result_(std::numeric_limits<double>::quiet_NaN(),
                     std::numeric_limits<double>::quiet_NaN()),
    histogramplot_cache_(nullptr)
{
    DetermineOutmostZoomIfZoomStackIsEmpty();
}

double HistogramData2D::CalcXMean() const
{
    return x_data_->CalcMean(GetMask());
}

double HistogramData2D::CalcYMean() const
{
    return y_data_->CalcMean(GetMask());
}

double HistogramData2D::CalcXStdDev() const
{
    return x_data_->CalcStdDev(GetMask());
}

double HistogramData2D::CalcYStdDev() const
{
    return y_data_->CalcStdDev(GetMask());
}

double HistogramData2D::CalcCorrelation() const
{
    return x_data_->CalcCorrelation(*y_data_, GetMask());
}


QwtMatrixRasterData* HistogramData2D::GetHistogramRasterData() const
{
    assert(!zoom_stack_.empty());

    if (IsCacheInvalid())
    {
        const double min_x = zoom_stack_.top().x.minValue();
        const double max_x = zoom_stack_.top().x.maxValue();
        const double min_y = zoom_stack_.top().y.minValue();
        const double max_y = zoom_stack_.top().y.maxValue();
        const unsigned bins = GetCurrentBinNumber();
        const double width_x = (max_x - min_x) / bins;
        const double width_y = (max_y - min_y) / bins;

        QVector<double> samples(bins * bins, -2.);

        unsigned highest_number_of_samples_in_bin = 0U;

        MaskForEach(*x_data_->vector_, *y_data_->vector_, GetMask(),
            [&](const double& x, const double& y)
            {
                if (!zoom_stack_.top().x.contains(x) ||
                    !zoom_stack_.top().y.contains(y))
                    return;
                unsigned index_x = (x - min_x) / width_x;
                unsigned index_y = (y - min_y) / width_y;
                if (index_x >= bins) index_x = bins - 1;
                if (index_y >= bins) index_y = bins - 1;
                unsigned index = index_x + bins * index_y;
                if (++samples[index] < 0)
                    samples[index] = 1.0;
                if (samples[index] > highest_number_of_samples_in_bin)
                    highest_number_of_samples_in_bin = samples[index];
            });

        delete histogramplot_cache_;
        histogramplot_cache_ = new QwtMatrixRasterData();
        histogramplot_cache_->setValueMatrix(samples, bins);
        histogramplot_cache_->
                setResampleMode(QwtMatrixRasterData::NearestNeighbour);
        histogramplot_cache_->
                setInterval(Qt::XAxis, zoom_stack_.top().x);
        histogramplot_cache_->
                setInterval(Qt::YAxis, zoom_stack_.top().y);
        histogramplot_cache_->setInterval(
                Qt::ZAxis, QwtInterval(0, highest_number_of_samples_in_bin));
        MakeCacheValid();
    }

    return histogramplot_cache_;
}

void HistogramData2D::ZoomIn(QRectF rect)
{
    if (rect.isEmpty())
        return;
    zoom_stack_.emplace(rect.left(), rect.right(), rect.top(), rect.bottom());
    InvalidateCachedHistogram();
}

void HistogramData2D::ZoomOut()
{
    assert(!zoom_stack_.empty());
    if (zoom_stack_.size() == 1)
        return;
    zoom_stack_.pop();
    InvalidateCachedHistogram();
}

const QwtInterval& HistogramData2D::GetXInterval() const
{
    assert(!zoom_stack_.empty());
    return zoom_stack_.top().x;
}

const QwtInterval& HistogramData2D::GetYInterval() const
{
    assert(!zoom_stack_.empty());
    return zoom_stack_.top().y;
}

void HistogramData2D::MaskWithSelection()
{
    const QPolygonF& polygon = selection_polygon_;
    auto& mask = BeginEditMask();
    assert(mask.size() == x_data_->vector_->size());
    assert(mask.size() == y_data_->vector_->size());
    for (unsigned long i = 0; i < x_data_->vector_->size(); ++i)
        mask[i] = polygon.containsPoint(QPointF((*x_data_->vector_)[i],
                                                (*y_data_->vector_)[i]),
                                        Qt::OddEvenFill)
                  != IsSelectionInverted()
                  && mask[i];
    EndEditMask();
}

const QPolygonF& HistogramData2D::GetSelectionPolygon() const
{
    return selection_polygon_;
}

void HistogramData2D::SetSelectionPolygon(const QPolygonF& polygon)
{
    selection_polygon_ = polygon;
    emit SelectionChanged();
}

void HistogramData2D::ClearSelection()
{
    selection_polygon_.clear();
    HistogramDataBase::ClearSelection();
}

const QPointF& HistogramData2D::GetOriginalResult() const
{
    return original_result_;
}

void HistogramData2D::SetOriginalResult(double x, double y)
{
    original_result_ = QPointF(x, y);
}

void HistogramData2D::DetermineOutmostZoomIfZoomStackIsEmpty()
{
    if (!zoom_stack_.empty())
        return;

    const auto& x = *x_data_;
    const auto& y = *y_data_;
    assert(x.vector_->size());
    assert(x.vector_->size() == y.vector_->size());
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    min_x = max_x = (*x.vector_)[0];
    min_y = max_y = (*y.vector_)[0];
    for (unsigned i = 0; i < x.vector_->size(); ++i)
    {
        if ((*x.vector_)[i] < min_x) min_x = (*x.vector_)[i];
        if ((*x.vector_)[i] > max_x) max_x = (*x.vector_)[i];
        if ((*y.vector_)[i] < min_y) min_y = (*y.vector_)[i];
        if ((*y.vector_)[i] > max_y) max_y = (*y.vector_)[i];
    }
    zoom_stack_.emplace(min_x, max_x, min_y, max_y);

    InvalidateCachedHistogram();
}
