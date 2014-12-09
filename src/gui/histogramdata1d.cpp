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


#include <cmath>

#include "histogramdata1d.h"

HistogramData1D::HistogramData1D(
            unsigned long size,
            const SharedBinNumber& n_bins_default,
            SharedMask mask,
            boost::shared_ptr<const DataVector> data) :
        HistogramDataBase(size, n_bins_default, mask),
        data_(data),
        original_result_(std::numeric_limits<double>::quiet_NaN()),
        histogramplot_cache_(nullptr)
{
    DetermineOutmostZoomIfZoomStackIsEmpty();
}

double HistogramData1D::CalcMean() const
{
    return data_->CalcMean(GetMask());
}

double HistogramData1D::CalcStdDev() const
{
    return data_->CalcStdDev(GetMask());
}

QwtIntervalSeriesData* HistogramData1D::GetHistogramData() const
{
    if (IsCacheInvalid())
    {
        const double min = zoom_stack_.top().minValue();
        const double max = zoom_stack_.top().maxValue();
        const unsigned bins = GetCurrentBinNumber();
        const double width = (max - min) / bins;

        QVector<QwtIntervalSample> samples(bins);
        for (unsigned i = 0; i < bins; ++i)
        {
            QwtInterval interval(min + i * width, min + (i + 1) * width);
            interval.setBorderFlags(QwtInterval::ExcludeMaximum);
            samples[i] = QwtIntervalSample(0, interval);
        }

        MaskForEach(*data_->vector_, GetMask(),
            [&](const double& x)
            {
                if (x > max || x < min) return;
                unsigned bin = (x - min) / width;
                if (bin >= bins) bin = bins - 1;
                ++samples[bin].value;
            });

        delete histogramplot_cache_;
        histogramplot_cache_ = new QwtIntervalSeriesData(samples);
        MakeCacheValid();
    }

    return histogramplot_cache_;
}

void HistogramData1D::ZoomIn(QRectF rect)
{
    if (rect.isEmpty())
        return;
    zoom_stack_.emplace(rect.left(), rect.right());
    InvalidateCachedHistogram();
}

void HistogramData1D::ZoomOut()
{
    assert(!zoom_stack_.empty());
    if (zoom_stack_.size() == 1)
        return;
    zoom_stack_.pop();
    InvalidateCachedHistogram();
}

void HistogramData1D::DetermineOutmostZoomIfZoomStackIsEmpty()
{
    if (!zoom_stack_.empty())
        return;

    auto minmax = std::minmax_element(data_->vector_->cbegin(),
                                      data_->vector_->cend());
    zoom_stack_.emplace(*minmax.first, *minmax.second);

    InvalidateCachedHistogram();
}

void HistogramData1D::MaskWithSelection()
{
    const QwtInterval& interval = selection_interval_;
    auto& mask = BeginEditMask();
    assert(mask.size() == data_->vector_->size());
    for (unsigned long i = 0; i < data_->vector_->size(); ++i)
        mask[i] = interval.contains((*data_->vector_)[i])
                  != IsSelectionInverted()
                  && mask[i];
    EndEditMask();
}

const QwtInterval& HistogramData1D::GetSelectionInterval() const
{
    return selection_interval_;
}

void HistogramData1D::SetSelectionInterval(const QwtInterval& interval)
{
    selection_interval_ = interval;
    emit SelectionChanged();
}

void HistogramData1D::ClearSelection()
{
    selection_interval_.invalidate();
    HistogramDataBase::ClearSelection();
}

double HistogramData1D::GetOriginalResult() const
{
    return original_result_;
}

void HistogramData1D::SetOriginalResult(double result)
{
    original_result_ = result;
}
